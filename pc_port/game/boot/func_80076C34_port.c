/*
 * Phase 6E-B53E — honest prefix of the libgpu command dispatcher
 * func_80076C34 through the translated I_MASK exchange func_80073E10,
 * plus its inseparable timeout initializer func_800773D0.
 *
 * Retail func_80076C34 is 172 words / 0x2B0 bytes at
 * 0x80076C34..0x80076EE3 (exclusive end 0x80076EE4).  Its executable body
 * SHA-256 is b3686b34851b08fa3bb0097263caf59519056417b593606b7e7a59c155b1e508.
 * B53E resolves the exact canonical worker identity func_80076664 through
 * its translated LoadImage issue body, then restores the saved I_MASK and
 * returns retail zero.  Other worker identities remain honest boundaries.
 * Forced enqueue states still stop at callback registration helper
 * func_80073CF4 (call at 0x80076D60); a full ring still stops at
 * func_80077404 (call at 0x80076C68).  No ring entry is constructed or
 * published and no pump/callback runs.
 *
 * func_800773D0 is the complete 13-word timeout helper at
 * 0x800773D0..0x80077403.  It queries VSync(-1), stores query+240 at
 * D_80095888, clears D_8009588C, and returns the deadline.  B53B's inert,
 * deterministic GPU VSync counter is the proven hardware authority.
 *
 * func_80073E10 is the complete 6-word I_MASK exchange at
 * 0x80073E10..0x80073E27, translated in platform/pe_libetc.c against the
 * single PE_IRQ authority.  The prefix consumes its real previous-mask
 * return exactly as retail does: a 32-bit store to D_8009587C at
 * 0x80076CB4.
 *
 * No ring entry, payload, producer, consumer, callback identity, worker
 * result, or GPU/DMA progress is mirrored in native state.  The prefix
 * only reads the authoritative producer/consumer words in guest RAM and
 * writes the two proven retail words D_8009587C / D_80095754.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"
#include <stdio.h>

#define GA_GPU_INIT_BYTE         0x8009574Du
#define GA_GPU_WORK_MARKER       0x80095754u
#define GA_GPU_DRAWSYNC_CALLBACK 0x80095758u
#define GA_GPU_RING_PRODUCER     0x80095874u
#define GA_GPU_RING_CONSUMER     0x80095878u
#define GA_GPU_SAVED_IMASK       0x8009587Cu
#define GA_GPU_TIMEOUT_DEADLINE  0x80095888u
#define GA_GPU_TIMEOUT_POLLS     0x8009588Cu

#define GPU_DMA2_CHCR_BUSY       0x01000000u
#define GPU_QUEUE_PUMP           0x80076EE4u
#define GPU_SET_DMA_CALLBACK     0x80073CF4u

uint32_t func_800773D0(void)
{
    uint32_t deadline = PE_GPU_VSyncQuery() + 0xF0u;

    PE_StoreU32(GA_GPU_TIMEOUT_DEADLINE, deadline);
    PE_StoreU32(GA_GPU_TIMEOUT_POLLS, 0u);
    return deadline;
}

/* Diagnostic identity for the unresolved guest worker call.  Known
 * identities are the four retail command-issue workers from the B53A
 * caller census; any other 32-bit guest address is formatted by value.
 * The bootstrap registry keeps the returned pointer, so unknown names
 * rotate through static storage. */
static const char *worker_symbol(pe_addr_t worker)
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

static int func_80076C34_prefix(pe_addr_t worker, pe_addr_t argument,
                                int32_t copy_bytes, uint32_t auxiliary,
                                const uint32_t *inline8)
{
    uint32_t producer;
    uint32_t consumer;
    uint32_t next;
    uint32_t previous_mask;

    /* 0x80076C58 jal func_800773D0; the delay slot captures auxiliary.
     * Host arguments are already stable values, so the call is equivalent. */
    (void)copy_bytes;
    (void)func_800773D0();

    /* 0x80076C80..0x80076C98, in retail load/arithmetic order. */
    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
    next = (producer + 1u) & 63u;

    if (next == consumer) {
        /* 0x80076C68: the full-ring timeout helper is the first dependency
         * on this alternate path.  Its result is intentionally not used. */
        (void)Bootstrap_ReturnInt(
            "func_80077404", "func_80076C34", 0);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0; /* host prefix cut; not a claimed retail result */
    }

    /* 0x80076CA0: jal func_80073E10, delay a0=0 — now the translated
     * 16-bit I_MASK exchange against the single PE_IRQ authority. */
    previous_mask = func_80073E10(0u);

    /* 0x80076CB4: the previous mask is published as a 32-bit word.
     * 0x80076CB8 loads the initialization byte; 0x80076CC4 is the branch
     * delay slot and always publishes the work-submitted marker. */
    PE_StoreU32(GA_GPU_SAVED_IMASK, previous_mask);
    {
        uint32_t initialized = PE_LoadU8(GA_GPU_INIT_BYTE);
        PE_StoreU32(GA_GPU_WORK_MARKER, 1u);
        if (initialized != 0u) {
            /* 0x80076CCC/0x80076CD4: retail reloads both indices after the
             * exchange; a nonempty ring selects the enqueue path. */
            producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
            consumer = PE_LoadU32(GA_GPU_RING_CONSUMER);
            if (producer == consumer &&
                (PE_GPU_ReadDMA2CHCR() & GPU_DMA2_CHCR_BUSY) == 0u &&
                PE_LoadU32(GA_GPU_DRAWSYNC_CALLBACK) == 0u) {
                goto direct_issue;
            }
            /* 0x80076D58..0x80076D64: enqueue begins by installing the pump
             * through func_80073CF4(2, 0x80076EE4).  Callback registration
             * is the first unresolved dependency on this path; no ring entry
             * is constructed or published before it. */
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80073CF4", "func_80076C34", 0,
                GPU_SET_DMA_CALLBACK, 2u, GPU_QUEUE_PUMP, 0u, 0u, NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0; /* host prefix cut; not a claimed retail result */
        }
    }
    /* Initialization byte zero: the 0x80076CC0 branch skips every queue,
     * DMA, and callback check straight to the readiness poll. */

direct_issue:
    /* 0x80076D20..0x80076D2C: unbounded retail GPUSTAT bit-26 tight poll.
     * The B53B authority never auto-progresses, so this loop terminates
     * exactly when the platform reports ready, as on retail hardware. */
    while ((PE_GPU_ReadStatus() & PE_GPU_STATUS_READY_GP0) == 0u) {
    }

    /* 0x80076D38: jalr s3 — the guest command-issue worker, called with
     * a0 = argument (s0) and a1 = auxiliary (s2, delay slot).  Resolve only
     * the exact LoadImage identity translated by B53E. */
    if (worker == 0x80076664u) {
        if (inline8) {
            (void)PE_func_80076664_Inline8(
                inline8[0], inline8[1], auxiliary);
        } else {
            (void)func_80076664(argument, auxiliary);
        }
        /* A nested unresolved timeout/recovery boundary has not returned in
         * retail.  Preserve that exact prefix rather than fabricating the
         * caller's mask restore. */
        if (PE_Port_GetStopReason() == PE_PORT_STOP_UNRESOLVED_BOUNDARY) {
            return 0;
        }
        /* 0x80076D40..0x80076D54: the worker result is deliberately ignored,
         * the saved low 16-bit I_MASK is restored, and direct issue returns
         * zero. */
        (void)func_80073E10((uint16_t)PE_LoadU32(GA_GPU_SAVED_IMASK));
        return 0;
    }

    (void)Bootstrap_ReturnInt4Indirect(
        worker_symbol(worker), "func_80076C34", 0,
        worker, argument, auxiliary, 0u, 0u, NULL, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0; /* host prefix cut; not a claimed retail result */
}

int func_80076C34(pe_addr_t worker, pe_addr_t argument,
                  int32_t copy_bytes, uint32_t auxiliary)
{
    return func_80076C34_prefix(
        worker, argument, copy_bytes, auxiliary, NULL);
}

int PE_func_80076C34_Inline8(pe_addr_t worker,
                             uint32_t argument_word0,
                             uint32_t argument_word1,
                             uint32_t auxiliary)
{
    const uint32_t payload[2] = { argument_word0, argument_word1 };

    /* B52's transient native RECT is converted to two values before this
     * point.  Keep its accepted call evidence without retaining a native
     * pointer or treating this diagnostic snapshot as queue authority. */
    Bootstrap_RecordArg4Indirect(
        "func_80076C34", "func_8007506C", 0x80076C34u,
        worker, 0u, 8u, auxiliary, payload, sizeof(payload));
    /* No guest argument identity exists for this caller-stack transient.
     * The exact LoadImage direct worker consumes these two words by value
     * before return; enqueue still stops at func_80073CF4 before any payload
     * must survive. */
    return func_80076C34_prefix(worker, 0u, 8, auxiliary, payload);
}
