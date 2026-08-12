/* Phase 6C — Game port shared declarations */
#ifndef GAME_PORT_H
#define GAME_PORT_H

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

void PE_Port_RunControlReset(void);
void PE_Port_SetFrameLimit(int frames);
void PE_Port_SetMainIterationLimit(int iterations);
void PE_Port_SetQuitPoll(PEPortQuitPoll poll);
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

/* Trace helper available to game code */
void Trace_Direct(const char *event);

#endif
