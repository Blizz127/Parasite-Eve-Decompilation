/*
 * Phase 6E-B53I-C — complete bounded translation of the libgpu command
 * queue pump func_80076EE4.
 *
 * Retail func_80076EE4 is 152 words / 0x260 bytes at
 * 0x80076EE4..0x80077143 (exclusive end 0x80077144, executable file offset
 * 0x676E4, live split asm/disc1/66B54.s).  Its body SHA-256 is
 * a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c.
 *
 * ABI: `int func_80076EE4(void)`.  No instruction in the body reads a0..a3
 * before writing it, and the four direct callers pass no arguments.
 *
 * B53H translated the entry busy-DMA fast path.  B53I-C adds the remaining
 * idle-DMA consumer at 0x80076F10..0x8007712F.  Queue state, callback
 * identities, the saved I_MASK, and the ring entries all remain solely in
 * guest RAM; GPU/DMA state remains solely in pe_gpu.
 *
 *   80076EE4  lui   v0,0x8009
 *   80076EE8  lw    v0,0x5860(v0)   ; D_80095860 = 0x1F8010A8 (DMA2 CHCR)
 *   80076EEC  addiu sp,sp,-0x20     ; private frame only
 *   80076EF0  sw    ra,0x18(sp)
 *   80076EF4  sw    s1,0x14(sp)
 *   80076EF8  sw    s0,0x10(sp)
 *   80076EFC  lw    v0,0(v0)        ; CHCR - first hardware state read
 *   80076F00  lui   s0,0x0100       ; 0x01000000 = CHCR bit 24 (start/busy)
 *   80076F04  and   v0,v0,s0
 *   80076F08  bnez  v0,0x80077130   ; busy -> straight to the epilogue
 *   80076F0C   addiu v0,zero,1      ; DELAY SLOT: the return value is 1
 *   ...
 *   80077130  lw    ra,0x18(sp)
 *   80077134  lw    s1,0x14(sp)
 *   80077138  lw    s0,0x10(sp)
 *   8007713C  jr    ra
 *   80077140   addiu sp,sp,0x20
 *
 * On this path the function therefore:
 *
 *   - reads DMA2 CHCR and nothing else;
 *   - never calls func_80073E10, so it performs NO I_MASK exchange and has
 *     no restore obligation (the exchange at 0x80076F10 is past the branch);
 *   - never reads the producer D_80095874 or the consumer D_80095878;
 *   - never reads or writes any ring entry;
 *   - never reads GPUSTAT (the first GPUSTAT read is at 0x80076FA4);
 *   - never calls func_80073CF4/func_800746A0, so callback slot 2 and DICR
 *     are untouched (the deregistration call is at 0x80076F90);
 *   - never invokes a queued worker (the jalr is at 0x80077034);
 *   - never advances the consumer (the store is at 0x80077054);
 *   - never clears the work marker or invokes the DrawSync callback
 *     (0x80077108 / 0x8007710C);
 *   - writes only its own stack frame;
 *   - returns exactly 1.
 *
 * The busy path executes 16 of the 152 words: the 11-word prefix
 * 0x80076EE4..0x80076F0C plus the 5-word shared epilogue
 * 0x80077130..0x80077140.  The other 136 words are the idle consumer.  It
 * disables I_MASK, optionally removes the final DMA2 callback, polls the
 * literal GPUSTAT-ready bit, dispatches a guest ring worker, advances the
 * consumer only after an ordinary worker return, restores I_MASK, performs
 * the exact conditional DrawSync cleanup, and returns the live pending
 * count.  Unknown guest indirect identities remain honest typed boundaries.
 *
 * D_80095860 is a retail .data word holding the literal MMIO address
 * 0x1F8010A8.  CHCR itself stays in the single B53B pe_gpu authority; this
 * translation creates no second CHCR representation.
 *
 * B54K-R resolves worker `func_80076B98` only for its authenticated
 * one-packet MoveImage form.  A normal return advances the consumer through
 * the existing retail pump ordering; unsupported general linked lists stop
 * before consumer publication.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"
#include "pe_irq.h"

#include <stdio.h>
#include <string.h>

/* Retail .data word holding the DMA2 CHCR MMIO address literal. */
#define GA_GPU_DMA2_CHCR_POINTER 0x80095860u
#define PE_DMA2_CHCR_ADDRESS     0x1F8010A8u

#define GA_GPU_WORK_MARKER       0x80095754u
#define GA_GPU_DRAWSYNC_CALLBACK 0x80095758u
#define GA_GPU_RING_PRODUCER     0x80095874u
#define GA_GPU_RING_CONSUMER     0x80095878u
#define GA_GPU_PUMP_SAVED_IMASK  0x80095880u
#define GA_GPU_RING_BASE         0x800BD030u
#define GA_DMA_CALLBACK_SLOT2    0x800956C8u
#define GPU_RING_FIELD_WORKER    0x00u
#define GPU_RING_FIELD_ARGUMENT  0x04u
#define GPU_RING_FIELD_AUXILIARY 0x08u
#define GPU_QUEUE_MASK           63u

#define GPU_LOADIMAGE_WORKER     0x80076664u
#define GPU_LINKED_LIST_WORKER   0x80076B98u

/* Value-only entry telemetry used to prove that hardware completion and the
 * DICR edge bridge never call the pump, including its otherwise silent busy
 * fast path.  It is not queue, callback, or run-control authority. */
static uint64_t g_pump_entry_count;
static uint64_t g_pump_trace_order;
static PeGpuPumpTrace g_pump_trace;

static uint64_t PumpTraceNext(void)
{
    g_pump_trace_order++;
    if (g_pump_trace_order == 0u) g_pump_trace_order++;
    return g_pump_trace_order;
}

static const char *PumpWorkerSymbol(pe_addr_t worker)
{
    static char unknown[4][16];
    static unsigned next;

    switch (worker) {
    case 0x80076434u: return "func_80076434";
    case 0x80076664u: return "func_80076664";
    case 0x800768A0u: return "func_800768A0";
    case 0x80076B98u: return "func_80076B98";
    default: {
        char *name = unknown[next++ & 3u];
        snprintf(name, 16, "func_%08X", worker);
        return name;
    }
    }
}

/* Exact `((consumer << 1) + consumer) << 5` MIPS arithmetic. */
static uint32_t PumpRingOffset(uint32_t consumer)
{
    return (uint32_t)(((consumer << 1) + consumer) << 5);
}

/* Retail independently reloads the consumer for each of the three ring
 * fields.  Preserve that live-load behavior and validate only the exact
 * resulting U32 access before translating it to a host pointer. */
static int PumpLoadRingFieldAt(uint32_t consumer, uint32_t field,
                               uint32_t *value)
{
    pe_addr_t address = (pe_addr_t)(GA_GPU_RING_BASE +
                                    PumpRingOffset(consumer) + field);

    if (!PE_RangeIsRam(address, sizeof(uint32_t))) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076EE4_ring_span", "func_80076EE4", 0,
            address, consumer, field, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    *value = PE_LoadU32(address);
    return 1;
}

/* Return one only when the indirect worker returned in retail control
 * flow.  Its integer v0 is deliberately ignored by func_80076EE4. */
static int PumpDispatchWorker(pe_addr_t worker, pe_addr_t argument,
                              uint32_t auxiliary,
                              uint32_t consumer_at_worker_lookup)
{
    g_pump_trace.worker_calls++;
    g_pump_trace.worker_call_order = PumpTraceNext();
    g_pump_trace.last_worker = worker;
    g_pump_trace.last_argument = argument;
    g_pump_trace.last_auxiliary = auxiliary;
    g_pump_trace.consumer_before_worker = consumer_at_worker_lookup;
    g_pump_trace.mask_at_worker = PE_IRQ_GetMask();
    g_pump_trace.dma_callback_at_worker =
        PE_LoadU32(GA_DMA_CALLBACK_SLOT2);
    g_pump_trace.dicr_at_worker = PE_GPU_ReadStoredDICR();
    g_pump_trace.chcr_at_worker = PE_GPU_ReadDMA2CHCR();

    if (worker == GPU_LOADIMAGE_WORKER || worker==0x800768A0u) {
        unsigned stop_epoch = PE_Port_StopEpoch();

        if (worker==0x800768A0u) (void)func_800768A0(argument,auxiliary);
        else (void)func_80076664(argument, auxiliary);
        if (PE_Port_StopEpoch() != stop_epoch) return 0;
        g_pump_trace.worker_returned = 1;
        return 1;
    }

    if (worker == GPU_LINKED_LIST_WORKER || worker == 0x80076434u) {
        unsigned stop_epoch = PE_Port_StopEpoch();

        if (worker == 0x80076434u) (void)func_80076434(argument,auxiliary);
        else (void)func_80076B98(argument, auxiliary);
        if (PE_Port_StopEpoch() != stop_epoch) return 0;
        g_pump_trace.worker_returned = 1;
        return 1;
    }

    /* Retail does not treat zero specially: jalr zero is still a bad guest
     * indirect call, not an empty slot. */
    (void)Bootstrap_ReturnInt4Indirect(
        PumpWorkerSymbol(worker), "func_80076EE4", 0,
        worker, argument, auxiliary, 0u, 0u, NULL, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

static int PumpDispatchDrawSync(pe_addr_t callback)
{
    g_pump_trace.drawsync_calls++;
    g_pump_trace.drawsync_call_order = PumpTraceNext();
    Bootstrap_ReturnVoid4Indirect(
        PumpWorkerSymbol(callback), "func_80076EE4", callback,
        0u, 0u, 0u, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

int PE_func_80076EE4_Pump(int *retail_returned)
{
    g_pump_entry_count++;
    if (retail_returned != NULL) {
        *retail_returned = 0;
    }

    /* 0x80076EFC..0x80076F08: the busy test is `CHCR & 0x01000000`, taken
     * before any queue or interrupt state is touched. */
    if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u) {
        /* 0x80076F0C delay slot, then the shared epilogue at 0x80077130. */
        if (retail_returned != NULL) {
            *retail_returned = 1;
        }
        return 1;
    }

    {
        uint32_t saved_mask;
        uint32_t producer;
        uint32_t consumer;
        uint32_t result;

        /* 0x80076F10..0x80076F30.  The producer and consumer loads precede
         * the saved-mask store in the branch delay slot. */
        saved_mask = func_80073E10(0u);
        g_pump_trace.saved_mask = saved_mask;
        g_pump_trace.mask_disable_order = PumpTraceNext();
        producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
        consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
        PE_StoreU32(GA_GPU_PUMP_SAVED_IMASK, saved_mask);
        if (producer == consumer) goto restore_mask;

        /* 0x80076F34..0x80076F50: DMA could have become busy before the
         * masked queue scan; that ordinary path restores the mask. */
        if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u) {
            goto restore_mask;
        }

        for (;;) {
            uint32_t next;
            uint32_t drawsync;
            uint32_t argument_word;
            uint32_t auxiliary_word;
            uint32_t worker_word;
            uint32_t argument_consumer;
            uint32_t auxiliary_consumer;
            uint32_t worker_consumer;

            /* 0x80076F5C..0x80076F94: only the final queued entry with no
             * DrawSync callback removes DMA callback slot 2. */
            consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
            next = (consumer + 1u) & GPU_QUEUE_MASK;
            if (next == producer) {
                drawsync = PE_LoadU32(GA_GPU_DRAWSYNC_CALLBACK);
                if (drawsync == 0u) {
                    (void)func_80073CF4(2u, 0u);
                    g_pump_trace.callback_removal_order = PumpTraceNext();
                }
            }

            /* 0x80076F98..0x80076FC8: literal unbounded GPUSTAT bit-26
             * polling.  Reads never evolve hardware or complete DMA. */
            for (;;) {
                uint32_t status = PE_GPU_ReadStatus();
                g_pump_trace.gpustat_reads++;
                if ((status & PE_GPU_STATUS_READY_GP0) != 0u) break;
            }
            g_pump_trace.gpustat_ready_order = PumpTraceNext();

            /* 0x80076FCC..0x80077034: the first consumer load feeds the
             * later worker lookup, the second feeds argument, and the third
             * feeds auxiliary.  All are live and intentionally uncached. */
            worker_consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            argument_consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            if (!PumpLoadRingFieldAt(argument_consumer,
                                     GPU_RING_FIELD_ARGUMENT,
                                     &argument_word)) {
                return 0;
            }
            auxiliary_consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            if (!PumpLoadRingFieldAt(auxiliary_consumer,
                                     GPU_RING_FIELD_AUXILIARY,
                                     &auxiliary_word) ||
                !PumpLoadRingFieldAt(worker_consumer,
                                     GPU_RING_FIELD_WORKER,
                                     &worker_word)) {
                return 0;
            }
            if (!PumpDispatchWorker((pe_addr_t)worker_word,
                                    (pe_addr_t)argument_word,
                                    auxiliary_word, worker_consumer)) {
                return 0;
            }

            /* 0x8007703C..0x80077054: the sole consumer store occurs only
             * after the worker's ordinary return. */
            consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            consumer = (consumer + 1u) & GPU_QUEUE_MASK;
            PE_StoreU32(GA_GPU_RING_CONSUMER, consumer);
            g_pump_trace.consumer_after_worker = consumer;
            g_pump_trace.consumer_advance_order = PumpTraceNext();

            /* 0x80077058..0x80077090: drain another entry only while the
             * ring remains nonempty and DMA2 is still idle. */
            producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
            consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            if (producer == consumer) break;
            if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u) {
                break;
            }
        }

restore_mask:
        /* 0x80077094..0x800770A0: every ordinary path after the exchange
         * restores the exact saved low 16-bit mask. */
        saved_mask = PE_LoadU32(GA_GPU_PUMP_SAVED_IMASK);
        (void)func_80073E10((uint16_t)saved_mask);
        g_pump_trace.restored_mask = (uint16_t)saved_mask;
        g_pump_trace.mask_restore_order = PumpTraceNext();

        /* 0x800770A4..0x80077110: DrawSync cleanup occurs only with an
         * empty ring, idle DMA2, set work marker, and nonzero callback.
         * The marker clear is committed before the guest indirect call. */
        producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
        consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
        if (producer == consumer &&
            (PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) == 0u &&
            PE_LoadU32(GA_GPU_WORK_MARKER) != 0u) {
            pe_addr_t callback =
                (pe_addr_t)PE_LoadU32(GA_GPU_DRAWSYNC_CALLBACK);
            if (callback != 0u) {
                PE_StoreU32(GA_GPU_WORK_MARKER, 0u);
                g_pump_trace.drawsync_clear_order = PumpTraceNext();
                if (!PumpDispatchDrawSync(callback)) return 0;
            }
        }

        /* 0x80077114..0x8007712C: live pending count, followed by the
         * shared epilogue. */
        producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
        consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
        result = (producer - consumer) & GPU_QUEUE_MASK;
        if (retail_returned != NULL) *retail_returned = 1;
        return (int)result;
    }
}

/* Retail-ABI form, and the identity registered in DMA callback slot 2.
 *
 * CONTRACT: production callers must use PE_func_80076EE4_Pump and honour
 * `retail_returned`.  This form discards that signal, and on a nested
 * indirect boundary it returns 0 — which is also a legal retail
 * value.  B53I's DMA IRQ dispatcher therefore binds to the _Pump form and
 * consumes the explicit return signal. */
int func_80076EE4(void)
{
    return PE_func_80076EE4_Pump(NULL);
}

/* Read-only names for the retail authority this prefix uses.  They are
 * test/documentation evidence only: no production code consults the retail
 * pointer word D_80095860, and CHCR itself stays owned by B53B pe_gpu.
 * They deliberately do NOT carry the PE_GPU_ platform prefix, so nothing
 * here can be mistaken for a second CHCR authority. */
pe_addr_t PE_Pump_Dma2ChcrPointerAddress(void)
{
    return (pe_addr_t)GA_GPU_DMA2_CHCR_POINTER;
}

uint32_t PE_Pump_Dma2ChcrMmioAddress(void)
{
    return PE_DMA2_CHCR_ADDRESS;
}

void PE_Pump_TraceReset(void)
{
    g_pump_entry_count = 0u;
    g_pump_trace_order = 0u;
    memset(&g_pump_trace, 0, sizeof(g_pump_trace));
}

uint64_t PE_Pump_EntryCount(void)
{
    return g_pump_entry_count;
}

void PE_Pump_GetTrace(PeGpuPumpTrace *out)
{
    if (out != NULL) *out = g_pump_trace;
}
