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
 * B53F resolves the execution-proven installed-target path through the
 * callback-registration wrapper func_80073CF4 at 0x80076D60, B53G completes
 * it by translating the separate setter func_800746A0, and B53H translates
 * the pump's busy-DMA fast path so this dispatcher now returns its real
 * retail pending count instead of stopping.  The
 * enqueue block 0x80076D68..0x80076EA0 is therefore translated: it copies
 * the payload, writes the entry metadata, advances the producer, restores
 * the saved I_MASK, and calls the pump at 0x80076EA4.  While the first
 * transfer is in flight the pump returns 1 and consumes nothing; only its
 * untranslated idle-DMA path stops.  A full ring still stops at
 * func_80077404 (call at 0x80076C68).  No callback delivery, consumer
 * movement, or DMA completion happens here.
 *
 * B54K-R adds the exact `func_80076B98` MoveImage packet subset on both the
 * direct and queued paths.  It accepts only the five-word GP0(80h) packet
 * template used by `func_8007512C`; general DrawOTag linked lists remain a
 * named boundary and no guest command stream is interpreted here.
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

#define GPU_QUEUE_PUMP           0x80076EE4u

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

/* Retail command-ring entry geometry, proven by the executable words at
 * 0x80076D70..0x80076E7C: 64 entries of 96 bytes at D_800BD030, with
 * +0x00 worker identity, +0x04 argument, +0x08 auxiliary, and the copied
 * inline payload from +0x0C.  The ring stays authoritative in guest RAM;
 * no native mirror or host queue exists. */
#define GA_GPU_RING_BASE         0x800BD030u
#define GPU_RING_ENTRY_STRIDE    96u
#define GPU_RING_FIELD_ARGUMENT  0x04u
#define GPU_RING_FIELD_AUXILIARY 0x08u
#define GPU_RING_FIELD_PAYLOAD   0x0Cu

/* `sll`+`addu`+`sll` at 0x80076DB0..0x80076DB8 in exact 32-bit guest
 * arithmetic: entry_offset = producer * 96. */
static uint32_t ring_entry_offset(uint32_t producer)
{
    return (uint32_t)(((producer << 1) + producer) << 5);
}

/* Preflight for the spans this enqueue would touch.  Retail performs no
 * check at all; it would simply read or write whatever address its
 * arithmetic produced.  This port cannot represent an access outside its
 * 2 MiB guest RAM, so — exactly as B53E does before the LoadImage CPU
 * prefix — an unrepresentable span becomes a visible honest boundary
 * instead of a host abort or a silently narrowed copy. */
static int enqueue_spans_are_ram(pe_addr_t argument, int32_t words,
                                 uint32_t producer, int has_inline_payload)
{
    uint64_t bytes;

    if (!PE_RangeIsRam(GA_GPU_RING_BASE + ring_entry_offset(producer),
                       GPU_RING_ENTRY_STRIDE)) {
        return 0;
    }
    if (words <= 0) {
        return 1;
    }
    bytes = (uint64_t)(uint32_t)words * 4u;
    if (bytes > GPU_RING_ENTRY_STRIDE - GPU_RING_FIELD_PAYLOAD) {
        return 0;   /* retail would overrun the following ring entries */
    }
    if (has_inline_payload) {
        return bytes <= 8u;
    }
    return PE_RangeIsRam(argument, (size_t)bytes);
}

/* 0x80076D68..0x80076EA8 — construct and publish one ring entry, then
 * expose the retail queue pump.
 *
 * Retail reloads D_80095874 before every entry store; that exact read
 * pattern is preserved because the ring word is authoritative guest state,
 * not a cached host value.  Producer publication happens in the
 * 0x80076EA0 delay slot, i.e. strictly before func_80073E10 restores the
 * saved I_MASK, and no store to the entry happens after it. */
static int func_80076C34_enqueue(pe_addr_t worker, pe_addr_t argument,
                                 int32_t copy_bytes, uint32_t auxiliary,
                                 const uint32_t *inline8)
{
    uint32_t producer;
    uint32_t saved_mask;

    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    if (!enqueue_spans_are_ram(argument, copy_bytes / 4, producer,
                               inline8 != NULL)) {
        (void)Bootstrap_ReturnInt(
            "func_80076C34_enqueue_span", "func_80076C34", 0);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    if (copy_bytes != 0) {
        /* 0x80076D80..0x80076D8C: the `bgez`/`addiu 3`/`sra 2` idiom is
         * signed division by four, and 0x80076D90 compares signed, so a
         * negative byte count copies nothing. */
        int32_t words = copy_bytes / 4;
        int32_t index;
        pe_addr_t source = argument;

        for (index = 0; index < words; index++) {
            uint32_t value;

            if (inline8 != NULL) {
                /* B52's caller-stack RECT has no guest address.  Its exact
                 * eight proven bytes are supplied by value; the preflight
                 * has already rejected any wider copy, which would read
                 * retail memory this port cannot name. */
                value = inline8[index];
            } else {
                value = PE_LoadU32(source);
            }
            source += 4u;

            producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
            PE_StoreU32(GA_GPU_RING_BASE + GPU_RING_FIELD_PAYLOAD +
                        ring_entry_offset(producer) +
                        (uint32_t)index * 4u, value);
        }

        /* 0x80076DD0..0x80076E10: the argument field becomes the guest
         * address of the copied payload, never a native pointer. */
        producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
        PE_StoreU32(GA_GPU_RING_BASE + GPU_RING_FIELD_ARGUMENT +
                    ring_entry_offset(producer),
                    GA_GPU_RING_BASE + GPU_RING_FIELD_PAYLOAD +
                    ring_entry_offset(producer));
    } else {
        /* 0x80076E14..0x80076E34: with no payload the caller's argument is
         * stored unchanged. */
        producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
        PE_StoreU32(GA_GPU_RING_BASE + GPU_RING_FIELD_ARGUMENT +
                    ring_entry_offset(producer), argument);
    }

    /* 0x80076E38..0x80076E58 then 0x80076E5C..0x80076E7C. */
    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    PE_StoreU32(GA_GPU_RING_BASE + GPU_RING_FIELD_AUXILIARY +
                ring_entry_offset(producer), auxiliary);
    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    PE_StoreU32(GA_GPU_RING_BASE + ring_entry_offset(producer), worker);

    /* 0x80076E80..0x80076EA0: the entry is complete before the producer
     * advances, and the producer advances before the mask restore. */
    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    saved_mask = PE_LoadU32(GA_GPU_SAVED_IMASK);
    PE_StoreU32(GA_GPU_RING_PRODUCER, (producer + 1u) & 63u);
    (void)func_80073E10((uint16_t)saved_mask);

    /* 0x80076EA4: jal func_80076EE4 — the opportunistic pump.  B53H
     * translates its execution-proven busy-DMA path; the return is
     * discarded here exactly as retail discards it. */
    {
        int pump_returned = 0;

        (void)PE_func_80076EE4_Pump(&pump_returned);
        if (!pump_returned) {
            /* The pump reached its untranslated idle-DMA consumer path and
             * has not returned in retail control flow.  Preserve that
             * prefix rather than fabricating the pending count.  This is an
             * explicit signal, not an inference from the stop reason, which
             * keeps only the first requested reason. */
            return 0;
        }
    }

    /* 0x80076EAC..0x80076EC4: the dispatcher recomputes its own pending
     * count from the authoritative ring words and returns it.  This is
     * reachable only when the pump really returned, and the count is
     * necessarily non-zero here: the space check above rejected
     * (producer+1)&63 == consumer, nothing between it and publication moves
     * either index, and the busy-DMA pump advances neither.  So it can
     * never be confused with the direct-issue path's retail zero. */
    producer = PE_LoadU32(GA_GPU_RING_PRODUCER);
    return (int)((producer - PE_LoadU32(GA_GPU_RING_CONSUMER)) & 63u);
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
         * on this alternate path.  Retail DOES consume its result:
         * 0x80076C70 `bnez v0` returns -1 from the delay slot, and a zero
         * result falls through to the pump at 0x80076C78 and re-runs the
         * space test.  Neither continuation is translated. */
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
                (PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) == 0u &&
                PE_LoadU32(GA_GPU_DRAWSYNC_CALLBACK) == 0u) {
                goto direct_issue;
            }
            /* 0x80076D58..0x80076D64: enqueue begins by calling the proven
             * installed-target path through func_80073CF4 with
             * (2, func_80076EE4).  B53G translates that setter completely,
             * so the real registration result is consumed here.  Retail
             * discards it (B53F caller census), and so does this call. */
            (void)func_80073CF4(2u, GPU_QUEUE_PUMP);
            return func_80076C34_enqueue(worker, argument, copy_bytes,
                                         auxiliary, inline8);
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
     * a0 = argument (s0) and a1 = auxiliary (s2, delay slot). */
    if (worker == 0x80076664u) {
        unsigned stop_epoch = PE_Port_StopEpoch();

        if (inline8) {
            (void)PE_func_80076664_Inline8(
                inline8[0], inline8[1], auxiliary);
        } else {
            (void)func_80076664(argument, auxiliary);
        }
        /* A nested unresolved timeout/recovery boundary has not returned in
         * retail.  Preserve that exact prefix rather than fabricating the
         * caller's mask restore.
         *
         * The test is an epoch comparison, not PE_Port_GetStopReason():
         * PE_Port_RequestStop keeps only the FIRST reason, so an already
         * latched stop would otherwise both hide a real boundary here and
         * make an earlier unrelated boundary skip this mandatory I_MASK
         * restore (retail 0x80076D40..0x80076D4C restores unconditionally). */
        if (PE_Port_StopEpoch() != stop_epoch) {
            return 0;
        }
        /* 0x80076D40..0x80076D54: the worker result is deliberately ignored,
         * the saved low 16-bit I_MASK is restored, and direct issue returns
         * zero. */
        (void)func_80073E10((uint16_t)PE_LoadU32(GA_GPU_SAVED_IMASK));
        return 0;
    }

    if (worker == 0x80076B98u) {
        unsigned stop_epoch = PE_Port_StopEpoch();

        (void)func_80076B98(argument, auxiliary);
        if (PE_Port_StopEpoch() != stop_epoch) {
            return 0;
        }
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
     * before return; the enqueue path copies them into the ring entry and
     * stores the entry's own guest payload address, so no native pointer
     * outlives this call. */
    return func_80076C34_prefix(worker, 0u, 8, auxiliary, payload);
}
