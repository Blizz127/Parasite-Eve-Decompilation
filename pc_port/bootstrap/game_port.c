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

int g_port_stop_requested = 0;
int g_port_main_iterations = 0;

static int g_port_frame_limit = 0;
static int g_port_main_iteration_limit = 0;
static int g_port_frame_budget_reached = 0;
static PEPortQuitPoll g_port_quit_poll = NULL;
static PEPortStopReason g_port_stop_reason = PE_PORT_STOP_NONE;
static int g_port_dma_irq_checkpoint_enabled = 1;

/* Monotonic count of stop requests.  PE_Port_RequestStop deliberately keeps
 * only the FIRST reason, so the reason value cannot tell a caller whether a
 * specific callee just requested a stop.  Callers that need that must
 * compare this epoch across the call instead. */
static unsigned g_port_stop_epoch;

void PE_Port_RunControlReset(void)
{
    g_port_stop_epoch = 0;
    g_port_stop_requested = 0;
    g_port_main_iterations = 0;
    g_port_frame_limit = 0;
    g_port_main_iteration_limit = 0;
    g_port_frame_budget_reached = 0;
    g_port_quit_poll = NULL;
    g_port_stop_reason = PE_PORT_STOP_NONE;
    g_port_dma_irq_checkpoint_enabled = 1;
}

void PE_Port_SetDmaIrqCheckpointEnabled(int enabled)
{
    g_port_dma_irq_checkpoint_enabled = enabled != 0;
}

int PE_Port_DmaIrqCheckpointEnabled(void)
{
    return g_port_dma_irq_checkpoint_enabled;
}

PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void)
{
    uint64_t dma_token;
    PeIrqGeneration irq_generation;
    int completed = 0;
    PeIrqEdgeResult edge;
    PeIrqServiceResult service;

    if (!g_port_dma_irq_checkpoint_enabled) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_IDLE;
    }

    /* Capture both authorities before admitting work.  A callback-created
     * DMA receives a different token and cannot be completed here because
     * this checkpoint never loops or recaptures. */
    dma_token = PE_GPU_DMA2Pending() ? PE_GPU_DMA2EventToken() : 0u;
    irq_generation = PE_IRQ_Generation();
    if (dma_token != 0u) {
        completed = PE_GPU_ServiceDMA2Completion(dma_token);
    }

    edge = PE_IRQ_BridgeDICRRisingEdge(irq_generation);
    if (edge == PE_IRQ_EDGE_STALE) {
        return PE_PORT_DMA_IRQ_CHECKPOINT_STALE;
    }

    service = PE_IRQ_ServicePendingForGeneration(irq_generation);
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
