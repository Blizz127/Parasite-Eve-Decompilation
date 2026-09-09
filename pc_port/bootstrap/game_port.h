/* Phase 6C — Game port shared declarations */
#ifndef GAME_PORT_H
#define GAME_PORT_H

#include <stdint.h>

/* Host-owned run-control state. */
extern int g_port_stop_requested;
extern int g_port_main_iterations;

typedef int (*PEPortQuitPoll)(void);

typedef enum PEPortStopReason {
    PE_PORT_STOP_NONE = 0,
    PE_PORT_STOP_EXPLICIT,
    PE_PORT_STOP_HOST_QUIT,
    PE_PORT_STOP_FRAME_LIMIT,
    PE_PORT_STOP_MAIN_ITERATION_LIMIT,
    PE_PORT_STOP_UNRESOLVED_BOUNDARY,
} PEPortStopReason;

typedef enum PEPortDmaIrqCheckpointResult {
    PE_PORT_DMA_IRQ_CHECKPOINT_IDLE = 0,
    PE_PORT_DMA_IRQ_CHECKPOINT_RETURNED,
    PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY,
    PE_PORT_DMA_IRQ_CHECKPOINT_STALE,
} PEPortDmaIrqCheckpointResult;

/* Value-only evidence for one-token checkpoint admission.  These counters
 * do not participate in scheduling or hardware authority. */
typedef struct PEPortDmaIrqCheckpointTrace {
    uint64_t checkpoint_calls;
    uint64_t token_queries;
    uint64_t service_calls;
    uint64_t last_captured_token;
    uint64_t last_serviced_token;
} PEPortDmaIrqCheckpointTrace;

void PE_Port_RunControlReset(void);
void PE_Port_SetFrameLimit(int frames);
void PE_Port_SetMainIterationLimit(int iterations);
void PE_Port_SetQuitPoll(PEPortQuitPoll poll);
/* Live-window present hook (Phase 6E-PRS1).  The host framebuffer invokes
 * the installed hook after counting each presentation so a windowed run
 * can blit the newest pixels without polling guest state.  The hook runs
 * on host data only and must never touch guest RAM/VRAM. */
typedef void (*PEPortPresentHook)(void);
void PE_Port_SetPresentHook(PEPortPresentHook hook);
void PE_Port_InvokePresentHook(void);
void PE_Port_RequestStop(PEPortStopReason reason);
int  PE_Port_BeginMainIteration(void);
int  PE_Port_ShouldStop(void);
int  PE_Port_FramePresentationAllowed(void);
void PE_Port_FramePresented(int presented);
PEPortStopReason PE_Port_GetStopReason(void);
/* Monotonic stop-request counter.  Compare it across a call to learn
 * whether that call requested a stop; PE_Port_GetStopReason cannot answer
 * that, because only the first reason is retained. */
unsigned PE_Port_StopEpoch(void);
const char *PE_Port_StopReasonName(PEPortStopReason reason);

/* Deterministic B53I-B2 hardware opportunity.  It captures at most one
 * already-active DMA token, then keeps completion, DICR-edge bridging, and
 * CPU IRQ service as separately callable phases. */
void PE_Port_SetDmaIrqCheckpointEnabled(int enabled);
int PE_Port_DmaIrqCheckpointEnabled(void);
PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void);
void PE_Port_DmaIrqCheckpointTraceReset(void);
void PE_Port_GetDmaIrqCheckpointTrace(PEPortDmaIrqCheckpointTrace *out);

/* Host dev entry (plan: skip the opening FMV to reach the field first).
 * When enabled, func_801909B4 does NOT invoke the movie driver
 * func_80192CE8(1); instead it returns the New-Game selector so the real
 * func_8006E9A0(1) publishes the field token 0xA80830C8 and the main loop
 * dispatches into the field tick.  This bypasses the untranslated title
 * menu and the STR/MDEC movie pipeline; it is a documented HOST_ADAPTED
 * shortcut, never a claim that the retail movie/title ran. */
void PE_Port_SetSkipMovie(int enabled);
int  PE_Port_SkipMovie(void);

/* Opt-in demo shortcut for M0010's EF(0) menu after Aya's profile.
 * Preserve existing defaults; the full 16F10/4DCA4 menu is not translated. */
void PE_Port_SetSkipOpeningMenu(int enabled);
int  PE_Port_SkipOpeningMenu(void);

/* Host pad fill for D_800BE9A2 (active-low Sony bits).  Retail writes
 * this from libpad/StartPAD at VSync; the port has no SIO, so a source
 * installed here is polled at the field-tick pad site (before 3EB04).
 * Windowed runs install HostWindow_PadRaw; tests/headless may install a
 * scripted source.  NULL source leaves guest RAM unchanged except the
 * existing idle-zero → 0xFFFF normalize. */
typedef uint16_t (*PEPortPadSource)(void);
void PE_Port_SetPadSource(PEPortPadSource source);
int  PE_Port_HasPadSource(void);
uint16_t PE_Port_ReadPadRaw(void);

/* Stage-1b fixture promote arm for func_801924F8's E0 pump.
 * Retail last-chunk sets D_800B89F4 before DMA3 → 7C214; fixtures plant a
 * body then call 7A214, which clears B89F4. Arming here survives that clear
 * so the pump can publish status 2 once without unconditional live promote.
 * Live Disc1 never arms this — it waits for B89F4 / VBlank. */
void PE_Port_ArmStreamPromote(void);
int  PE_Port_ConsumeStreamPromote(void);

/* Stage-1b complete-frame latch for got_frame C89C admission.
 * Set by func_8007C214 when D_800B89F4==1 (retail last video chunk —
 * E0 promote or 7C564 during PumpCdProgress), or by tests that
 * simulate that publication. Demuxed bodies at s1 are VLC bitstreams —
 * they do NOT start with STR magic 0x80010160 (that lives in the
 * 32-byte sector header). Pad-exit probes still cover synthetic plants;
 * this latch covers live frames. */
void PE_Port_NoteStreamFrameReady(void);
int  PE_Port_TakeStreamFrameReady(void);

/* Trace helper available to game code */
void Trace_Direct(const char *event);

#endif
