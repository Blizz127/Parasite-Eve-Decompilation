/*
 * Phase 6E-B49 — Explicit host run-control policy.
 *
 * Retail framebuffer presentation is not a termination condition.  The host
 * may request termination explicitly (for example, window close), or a test
 * may install an exact frame/main-iteration budget.  func_8001220C polls this
 * policy only at the host-safe continuation points already present in its
 * native adaptation.
 */

#include "game_port.h"
#include "pe_gpu.h"
#include "pe_irq.h"
#include "pe_irq_delivery.h"
#include <stddef.h>
#include <string.h>

int g_port_stop_requested = 0;
int g_port_main_iterations = 0;

static int g_port_frame_limit = 0;
static int g_port_main_iteration_limit = 0;
static int g_port_frame_budget_reached = 0;
static PEPortQuitPoll g_port_quit_poll = NULL;
static PEPortPresentHook g_port_present_hook = NULL;
static PEPortStopReason g_port_stop_reason = PE_PORT_STOP_NONE;
static int g_port_dma_irq_checkpoint_enabled = 1;
static PEPortDmaIrqCheckpointTrace g_port_dma_irq_checkpoint_trace;

/* Monotonic count of stop requests.  PE_Port_RequestStop deliberately keeps
 * only the FIRST reason, so the reason value cannot tell a caller whether a
 * specific callee just requested a stop.  Callers that need that must
 * compare this epoch across the call instead. */
static unsigned g_port_stop_epoch;
static int g_port_skip_movie = 0;
static PEPortPadSource g_port_pad_source = NULL;

void PE_Port_SetSkipMovie(int enabled)
{
    g_port_skip_movie = enabled != 0;
}

int PE_Port_SkipMovie(void)
{
    return g_port_skip_movie;
}

void PE_Port_SetPadSource(PEPortPadSource source)
{
    g_port_pad_source = source;
}

int PE_Port_HasPadSource(void)
{
    return g_port_pad_source != NULL;
}

uint16_t PE_Port_ReadPadRaw(void)
{
    if (g_port_pad_source == NULL)
        return 0xFFFFu;
    return g_port_pad_source();
}

void PE_Port_RunControlReset(void)
{
    g_port_stop_epoch = 0;
    g_port_skip_movie = 0;
    g_port_pad_source = NULL;
    g_port_stop_requested = 0;
    g_port_main_iterations = 0;
    g_port_frame_limit = 0;
    g_port_main_iteration_limit = 0;
    g_port_frame_budget_reached = 0;
    g_port_quit_poll = NULL;
    g_port_present_hook = NULL;
    g_port_stop_reason = PE_PORT_STOP_NONE;
    g_port_dma_irq_checkpoint_enabled = 1;
    memset(&g_port_dma_irq_checkpoint_trace, 0,
           sizeof(g_port_dma_irq_checkpoint_trace));
}

void PE_Port_SetDmaIrqCheckpointEnabled(int enabled)
{
    g_port_dma_irq_checkpoint_enabled = enabled != 0;
}

int PE_Port_DmaIrqCheckpointEnabled(void)
{
    return g_port_dma_irq_checkpoint_enabled;
}

void PE_Port_DmaIrqCheckpointTraceReset(void)
{
    memset(&g_port_dma_irq_checkpoint_trace, 0,
           sizeof(g_port_dma_irq_checkpoint_trace));
}

void PE_Port_GetDmaIrqCheckpointTrace(PEPortDmaIrqCheckpointTrace *out)
{
    if (out != NULL) {
        *out = g_port_dma_irq_checkpoint_trace;
    }
}

PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void)
{
    uint64_t dma_token;
    PeIrqGeneration irq_generation;
    int completed = 0;
    PeIrqEdgeResult edge = PE_IRQ_EDGE_NONE;
    PeIrqServiceResult service;

    g_port_dma_irq_checkpoint_trace.checkpoint_calls++;
    if (!g_port_dma_irq_checkpoint_enabled) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_IDLE;
    }

    /* Capture both authorities before admitting work.  A callback-created
     * DMA receives a different token and cannot be completed here because
     * this checkpoint never loops or recaptures. */
    dma_token = 0u;
    if (PE_GPU_DMA2Pending()) {
        g_port_dma_irq_checkpoint_trace.token_queries++;
        dma_token = PE_GPU_DMA2EventToken();
        g_port_dma_irq_checkpoint_trace.last_captured_token = dma_token;
    }
    irq_generation = PE_IRQ_Generation();
    if (dma_token != 0u) {
        g_port_dma_irq_checkpoint_trace.service_calls++;
        g_port_dma_irq_checkpoint_trace.last_serviced_token = dma_token;
        completed = PE_GPU_ServiceDMA2Completion(dma_token);
    }

    /* Channel-disabled completion creates no DICR edge, so it must not
     * enter even the inert edge bridge. */
    if (PE_GPU_DICRRisingEdgePending()) {
        edge = PE_IRQ_BridgeDICRRisingEdge(irq_generation);
    }
    if (edge == PE_IRQ_EDGE_STALE) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_STALE;
    }

    /* The hardware opportunity is not itself a CPU exception.  Enter the
     * retail scanner only when live I_STAT/I_MASK says a CPU source is
     * eligible.  This is deliberately not gated on the just-created DICR
     * edge: an older pending source that was later unmasked remains eligible.
     * Canonical B53I-D has no edge and I_STAT is zero, so completing the
     * interrupt-disabled second DMA does not call the CPU service at all. */
    service = PE_IRQ_SERVICE_RETURNED;
    if ((uint16_t)(PE_IRQ_ReadStatus() & PE_IRQ_GetMask()) != 0u) {
        service = PE_IRQ_ServicePendingForGeneration(irq_generation);
    }
    if (service == PE_IRQ_SERVICE_BOUNDARY) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY;
    }
    if (service == PE_IRQ_SERVICE_STALE) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_STALE;
    }
    if (completed || edge == PE_IRQ_EDGE_ASSERTED) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_RETURNED;
    }
    return PE_PORT_DMA_IRQ_CHECKPOINT_IDLE;
}

void PE_Port_SetFrameLimit(int frames)
{
    g_port_frame_limit = frames > 0 ? frames : 0;
    g_port_frame_budget_reached = 0;
}

void PE_Port_SetMainIterationLimit(int iterations)
{
    g_port_main_iteration_limit = iterations > 0 ? iterations : 0;
}

void PE_Port_SetQuitPoll(PEPortQuitPoll poll)
{
    g_port_quit_poll = poll;
}

void PE_Port_SetPresentHook(PEPortPresentHook hook)
{
    g_port_present_hook = hook;
}

void PE_Port_InvokePresentHook(void)
{
    if (g_port_present_hook) g_port_present_hook();
}

void PE_Port_RequestStop(PEPortStopReason reason)
{
    if (!g_port_stop_requested) {
        g_port_stop_reason = reason;
    }
    g_port_stop_requested = 1;
    g_port_stop_epoch++;
}

unsigned PE_Port_StopEpoch(void)
{
    return g_port_stop_epoch;
}

int PE_Port_BeginMainIteration(void)
{
    if (g_port_stop_requested) return 0;
    if (g_port_main_iteration_limit > 0 &&
        g_port_main_iterations >= g_port_main_iteration_limit) {
        PE_Port_RequestStop(PE_PORT_STOP_MAIN_ITERATION_LIMIT);
        return 0;
    }
    g_port_main_iterations++;
    return 1;
}

int PE_Port_ShouldStop(void)
{
    if (g_port_stop_requested) return 1;

    if (g_port_quit_poll && g_port_quit_poll()) {
        PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
        return 1;
    }

    if (g_port_frame_budget_reached) {
        PE_Port_RequestStop(PE_PORT_STOP_FRAME_LIMIT);
        return 1;
    }

    return 0;
}

int PE_Port_FramePresentationAllowed(void)
{
    return !g_port_frame_budget_reached;
}

void PE_Port_FramePresented(int presented)
{
    if (g_port_frame_limit > 0 && presented >= g_port_frame_limit) {
        g_port_frame_budget_reached = 1;
    }
}

PEPortStopReason PE_Port_GetStopReason(void)
{
    if (g_port_stop_requested && g_port_stop_reason == PE_PORT_STOP_NONE) {
        return PE_PORT_STOP_EXPLICIT;
    }
    return g_port_stop_reason;
}

const char *PE_Port_StopReasonName(PEPortStopReason reason)
{
    switch (reason) {
    case PE_PORT_STOP_NONE:                 return "none";
    case PE_PORT_STOP_EXPLICIT:             return "explicit";
    case PE_PORT_STOP_HOST_QUIT:            return "host-quit";
    case PE_PORT_STOP_FRAME_LIMIT:          return "frame-limit";
    case PE_PORT_STOP_MAIN_ITERATION_LIMIT: return "main-iteration-limit";
    case PE_PORT_STOP_UNRESOLVED_BOUNDARY:  return "unresolved-boundary";
    }
    return "unknown";
}
