/*
 * Phase 6E-B53C — honest prefix of the libgpu command dispatcher
 * func_80076C34, plus its inseparable timeout initializer func_800773D0.
 *
 * Retail func_80076C34 is 172 words / 0x2B0 bytes at
 * 0x80076C34..0x80076EE3 (exclusive end 0x80076EE4).  Its executable body
 * SHA-256 is b3686b34851b08fa3bb0097263caf59519056417b593606b7e7a59c155b1e508.
 * The prefix translated here follows the canonical non-full path through
 * the jal func_80073E10 and its delay slot at 0x80076CA0..0x80076CA4.  It
 * deliberately stops before consuming that helper's I_MASK result.
 *
 * func_800773D0 is the complete 13-word timeout helper at
 * 0x800773D0..0x80077403.  It queries VSync(-1), stores query+240 at
 * D_80095888, clears D_8009588C, and returns the deadline.  B53B's inert,
 * deterministic GPU VSync counter is the proven hardware authority.
 *
 * No ring entry, payload, producer, consumer, callback identity, worker
 * result, or GPU/DMA progress is mirrored in native state.  The prefix
 * only reads the authoritative producer/consumer words in guest RAM.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#define GA_GPU_RING_PRODUCER       0x80095874u
#define GA_GPU_RING_CONSUMER       0x80095878u
#define GA_GPU_TIMEOUT_DEADLINE    0x80095888u
#define GA_GPU_TIMEOUT_POLLS       0x8009588Cu

uint32_t func_800773D0(void)
{
    uint32_t deadline = PE_GPU_VSyncQuery() + 0xF0u;

    PE_StoreU32(GA_GPU_TIMEOUT_DEADLINE, deadline);
    PE_StoreU32(GA_GPU_TIMEOUT_POLLS, 0u);
    return deadline;
}

static int func_80076C34_prefix(pe_addr_t worker, int32_t copy_bytes,
                                uint32_t auxiliary)
{
    uint32_t producer;
    uint32_t consumer;
    uint32_t next;

    /* 0x80076C58 jal func_800773D0; the delay slot captures auxiliary.
     * Host arguments are already stable values, so the call is equivalent. */
    (void)worker;
    (void)copy_bytes;
    (void)auxiliary;
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
    } else {
        /* 0x80076CA0: jal func_80073E10, delay a0=0.  No faithful I_MASK
         * authority exists yet, so stop before using the returned mask. */
        (void)Bootstrap_ReturnInt1(
            "func_80073E10", "func_80076C34", 0, 0u);
    }

    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0; /* host prefix cut; not a claimed retail completion result */
}

int func_80076C34(pe_addr_t worker, pe_addr_t argument,
                  int32_t copy_bytes, uint32_t auxiliary)
{
    /* The guest argument identity remains 32-bit.  This prefix reaches no
     * instruction that reads it, so it is neither translated nor retained. */
    (void)argument;
    return func_80076C34_prefix(worker, copy_bytes, auxiliary);
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
    return func_80076C34_prefix(worker, 8, auxiliary);
}
