/*
 * Phase 6E-OTC1 — ClearOTagR (func_800752AC) plus its jtb[11] worker
 * func_80076354, translated retail logic with a synchronous OTC fill.
 *
 * Retail bodies (SHA-exact EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 *   func_800752AC: [0x800752AC,0x80075358), 43 words, asm/disc1/654C8.s.
 *     Debug name "ClearOTagR(%08x,%d)...\n" at D_80011910.
 *   func_80076354: [0x80076354,0x80076434), 56 words, asm/disc1/66B54.s.
 *     Reached only through jtb[11] (0x80076B98 census in
 *     game/boot/func_8007506C_port.c: jtb[11] = 0x80076354).
 *
 * func_800752AC body, ROM order:
 *   prologue; s0 = ot, s1 = n.
 *   1. lbu D_8009574E (SetGraphDebug level); level >= 2 prints
 *      ("ClearOTagR(%08x,%d)...", ot, n) through fn-ptr D_80095748
 *      (statically 0x80071A74). Boot/product level is 0: skip.
 *   2. v0 = *D_80095744 (jtb); jalr lw 0x2C(v0) = jtb[11] with (s0, s1).
 *   3. Tail (after the worker returns): D_8009580C =
 *      (0x800957F8 & 0xFFFFFF) | 0x400000 = 0x004957F8;
 *      *ot = 0x8009580C & 0xFFFFFF = 0x0009580C.
 *
 * func_80076354 body, ROM order: it programs DMA channel 6 (OTC,
 * "reverse clear OT"), whose registers are the static libgpu pointers
 *   D_80095864 -> 0x1F8010E0 (D6_MADR), D_80095868 -> 0x1F8010E4 (D6_BCR),
 *   D_8009586C -> 0x1F8010E8 (D6_CHCR), D_80095870 -> 0x1F8010F0 (DPCR).
 *   1. *D_80095870 |= 0x08000000 (DPCR DMA6 master enable).
 *   2. **D_8009586C = 0; *D_80095864 = ot + n*4 - 4 (LAST entry address);
 *      *D_80095868 = n.
 *   3. jal func_800773D0 (already translated: VSync query, deadline =
 *      query+0xF0 at D_80095888, polls = 0 at D_8009588C); delay slot
 *      **D_8009586C = 0x11000002 (CHCR: step -4, start/busy, enable).
 *   4. Wait loop: while (**D_8009586C & 0x01000000) { v0 = func_80077404();
 *      if (v0) break; } — 77404's ordinary path is an inert VSync query,
 *      deadline/poll-count checks, and polls++ (see WaitTimeoutPrefix in
 *      game/boot/func_80076664_port.c and PE_DrawSyncWaitPoll in
 *      platform/pe_libgpu.c).
 *
 * OTC hardware contract (PSX-SPX DMA chapter, channel 6 OTC: "each entry
 * points to the previous"; MADR = pointer to the LAST table entry, BCR =
 * entry count, CHCR = 11000002h): a count-N transfer at base B writes, for
 * every entry from the last down to the base, the 24-bit-masked address of
 * the previous entry, with the lowest (first) entry receiving the 0xFFFFFF
 * end marker that the GPU channel treats as its list terminator.  No GPU
 * rasterization, display-list walk, or callback is involved: the channel
 * writes terminators into guest RAM, which is why this rung has no host
 * question beyond completion itself.
 *
 * Native representation: the D6 register block has no other accessor in
 * the executable (only 66B54.s references D_80095864/68/6C/70), so no MMIO
 * traffic is modelled.  DPCR goes through the shared PE_GPU authority
 * (read-modify-write, composes with ResetGraph's 0x33333333 and the
 * channel-2 enable).  The fill itself is performed synchronously with the
 * PSX-SPX-derived hardware pattern (each entry points to the previous;
 * lowest entry takes the end marker — the only fill consistent with
 * MADR-last/BCR-count/CHCR and the GPU's 0xFFFFFF terminator), and the
 * virtual busy bit completes during the first wait poll — matching
 * real-hardware ordering (CHCR write, busy observed, wait, complete) with
 * a deterministic polls == 1.  Retail's exact iteration count is
 * bus-timing-dependent and not reproducible; the polls word is
 * diagnostic-only (consumed by 77404's 0xF0000 overflow check, six orders
 * of magnitude away).  This file is a behavior-verified native
 * translation, NOT a byte-exact matching leaf: no code was added under
 * src/, and none may be added until the docker era-gated rebuild
 * (scripts/build_us.sh + scripts/verify_us.sh) proves it word-exact.
 *
 * Null/unrepresentable-OT guard: retail always passes a real arena OT
 * (lookup[D_8009CDDC]; matched C leaf src/func_8006E9A0.c line 157), and
 * the de-adapted 6E9A0 loop does the same.  Writing an OTC pattern at
 * address 0 would corrupt the guest image base, so a null/unrepresentable
 * OT skips only the fill and tail, performs the safe non-memory effects
 * (DPCR RMW, 773D0 deadline/polls), and records a visible logged skip
 * WITHOUT a stop.  The skip is observable in the stub-order log, not
 * silent.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#define GA_GPU_DEBUG_LEVEL  0x8009574Eu
#define GA_GPU_PRINT_FN     0x80095748u
#define GA_GPU_JTB_PTR      0x80095744u
#define GA_GPU_PRINT_NAME   0x80011910u   /* "ClearOTagR(%08x,%d)...\n" */
#define GA_OTC_WORKER       0x80076354u   /* jtb[11] */
#define GA_GPU_DPCR_BIT27   0x08000000u
#define GA_GPU_CHCR_START   0x11000002u
#define GA_GPU_CHCR_BUSY    0x01000000u
#define GA_OTC_END_MARKER   0x00FFFFFFu
#define GA_OTC_ADDR_MASK    0x00FFFFFFu
#define GA_STUB_NODE_ADDR   0x800957F8u
#define GA_STUB_NODE_SLOT   0x8009580Cu
#define GA_OTC_FLAG_BIT     0x00400000u
#define GA_WAIT_POLL_LIMIT  0x000F0000u

/* Virtual D6 CHCR busy bit.  Set at program time, cleared when the
 * synchronous fill completes during the first wait poll.  No other
 * translation unit touches D6, so this is single-owner state. */
static uint32_t s_otc6_chcr = 0u;

/* Ordinary path of func_80077404's wait poll (PCs 0x80077404..0x80077454):
 * inert VSync query, signed deadline comparison, one exact prior-count
 * test, polls++.  Returns 0 to keep waiting, nonzero to exit the loop.
 * The timeout/recovery suffix at 0x80077458 stays a named boundary. */
static int OTC1_WaitPoll(void)
{
    uint32_t now = PE_GPU_VSyncQuery();
    uint32_t deadline = PE_LoadU32(0x80095888u);
    uint32_t polls = PE_LoadU32(0x8009588Cu);

    /* Retail stores polls+1 BEFORE the limit test (the sw lands ahead of
     * the slt/beqz), so the increment happens even on the recovery path. */
    PE_StoreU32(0x8009588Cu, polls + 1u);
    if ((int32_t)deadline < (int32_t)now ||
        (int32_t)polls > (int32_t)GA_WAIT_POLL_LIMIT) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80077404_timeout_recovery_cut", "func_80076354", -1,
            0x80077458u, deadline, now, polls, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    return 0;
}

void func_80076354(pe_addr_t ot, int n)
{
    uint32_t base;
    int32_t i;

    /* 0x80076370..0x8007637C: DPCR DMA6 master enable RMW. */
    PE_GPU_WriteDPCR(PE_GPU_ReadDPCR() | GA_GPU_DPCR_BIT27);

    /* Synchronous OTC fill with the exact hardware pattern, in retail
     * order: the transfer runs from the last entry down to the base while
     * the channel is busy.  Entry i > 0 receives the 24-bit-masked address
     * of entry i - 1; entry 0 receives the end marker (overwritten by the
     * 752AC tail after the wait loop, exactly as retail orders it). */
    base = ot;
    if (n > 0 && PE_RangeIsRam(base, (size_t)n * 4u)) {
        for (i = n - 1; i >= 1; i--)
            PE_StoreU32(base + (uint32_t)i * 4u,
                        (base + (uint32_t)(i - 1) * 4u) & GA_OTC_ADDR_MASK);
        PE_StoreU32(base, GA_OTC_END_MARKER);
    }
    s_otc6_chcr = GA_GPU_CHCR_START;

    /* 0x800763BC..0x800763C8: jal func_800773D0; delay slot programs CHCR. */
    (void)func_800773D0();

    /* 0x800763CC..0x8007641C: wait loop.  The fill completes during the
     * first wait poll (real-hardware ordering), so the second busy read
     * exits.  polls == 1 deterministically. */
    for (;;) {
        int wait_result;
        if ((s_otc6_chcr & GA_GPU_CHCR_BUSY) == 0u)
            break;
        wait_result = OTC1_WaitPoll();
        s_otc6_chcr &= ~GA_GPU_CHCR_BUSY;
        if (wait_result != 0)
            break;
    }
}

void func_800752AC(pe_addr_t ot, int n)
{
    pe_addr_t jtb;
    pe_addr_t target;

    /* 0x800752AC..0x800752F0: debug print only at level >= 2. */
    if (PE_LoadU8(GA_GPU_DEBUG_LEVEL) >= 2u) {
        pe_addr_t print_target = PE_LoadU32(GA_GPU_PRINT_FN);
        Bootstrap_ReturnVoid4Indirect(
            "func_80071A74", "func_800752AC", print_target,
            GA_GPU_PRINT_NAME, ot, (uintptr_t)(intptr_t)n, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }

    /* 0x800752F4..0x8007530C: jtb[11] dispatch with (ot, n).  The pointer
     * read is range-checked first: PE_LoadU32 aborts on unmapped addresses
     * and retail's zero-jtb crash is not a portable behavior. */
    jtb = PE_LoadU32(GA_GPU_JTB_PTR);
    if (!PE_RangeIsRam(jtb, 0x30u))
        target = 0u;
    else
        target = PE_LoadU32(jtb + 0x2Cu);
    if (target != GA_OTC_WORKER) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076354", "func_800752AC", 0, target,
            ot, (uintptr_t)(intptr_t)n, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    func_80076354(ot, n);

    /* Adapter-context skip (see header): no memory writes without a
     * representable OT.  Logged, not silent; no stop. */
    if (n <= 0 || !PE_RangeIsRam(ot, (size_t)n * 4u)) {
        Bootstrap_ReturnVoid("func_800752AC_null_ot_skip", "func_8006E9A0");
        return;
    }

    /* 0x80075310..0x80075340: terminator tail.  Computed from the constant
     * stub-node address exactly as retail masks it (not hardcoded). */
    PE_StoreU32(GA_STUB_NODE_SLOT,
                (GA_STUB_NODE_ADDR & GA_OTC_ADDR_MASK) | GA_OTC_FLAG_BIT);
    PE_StoreU32(ot, GA_STUB_NODE_SLOT & GA_OTC_ADDR_MASK);
}
