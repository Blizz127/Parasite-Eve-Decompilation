/*
 * Phase 6E-A — streaming engine bring-up (func_8006A5BC's four setup calls).
 *
 * ROM evidence (asm/disc1/74FB0.s, 75F44.s, 7D284.s):
 *   func_80085644 — streaming bring-up sequence
 *   func_80085290 — bulk streaming state init (242 asm lines, transcribed)
 *   func_8008CB08 — command-ring allocator (0x800B8628 + D_8009D2F4*0x24)
 *   func_8008CBA8 — streaming command dispatcher
 *   func_80086FF8/func_80087024/func_8008682C — command issuers
 *   func_80085A04/85F14/85EB4/850C0/850F4/85E54/85174/85F44/85098 — leaves
 * Classification: 1 for the guest-state transcription; the SPU/DMA hardware
 * effects are class 2 collapses enumerated below.
 *
 * Collapsed retail effects (hardware-only or degenerate on host):
 *   - func_8007D9F8 (SPU DMA upload in func_80085E54): hardware transfer.
 *     Completion is applied SYNCHRONOUSLY via the func_80085098 callback
 *     effects (func_80085F44(0) -> D_8009B434=0, D_8009D24C=0), so the
 *     func_80085174 spin terminates immediately — matching the retail
 *     post-completion state.
 *   - func_8007DB24(-1) (SPU heap query in func_80085EB4): host SPU-heap
 *     model starts at 0x1010 after SsInit (deterministic); only the store
 *     to D_8009B414 is guest-visible and the caller discards the return.
 *   - func_80085F74 / func_800862F4 (voice attribute/parameter setters in
 *     func_80085290): write through the voice-table base D_8009B3FC, which
 *     is installed by the collapsed SPU hardware init; not reproducible on
 *     host (same situation as D_8009B784 in pe_save.c).
 *   - func_8008CB54/func_8008CF70/func_80085A64 (streaming-mode command +
 *     SPU voice key off/on in func_80085290): SPU command-sequencer
 *     hardware effects consumed only by live audio streaming.
 *   - func_80085C44 (SPU control-register wait with "SPU:T/O" diagnostic),
 *     func_80085DC4 (SPU transfer-mode sequence in func_80085D84).
 *   - func_80085814/func_800858E8 retry loops (DMA acquire/release): their
 *     state tables D_8009B7CC/D_8009B7D0/D_8009B7D4 have no writer outside
 *     collapsed libsnd internals; modeled as host-synchronous (first poll
 *     succeeds).  The OpenEvent/EnableEvent retry loops use the real host
 *     event shims and also terminate on the first iteration.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "stub_registry.h"
#include <stdio.h>
#include <stdlib.h>

/* ── func_80085290 — bulk streaming state init (verbatim transcription) ── */
static void PE_Stream_StateInit(void)
{
    int i;
    pe_addr_t s0, v1;

    PE_StoreU32(0x8009D2C8u, 0x800B6980u);
    PE_StoreU32(0x800B6A30u, 0x7F0000u);
    PE_StoreU32(0x800B69C8u, 0x7F0000u);
    PE_StoreU32(0x8009D2B4u, 0x7FFF0000u);
    PE_StoreU32(0x8009D2C0u, 1);
    PE_StoreU32(0x800BCD50u, 0);
    PE_StoreU32(0x800B6984u, 0);
    PE_StoreU32(0x800B6988u, 0);
    PE_StoreU16(0x800B69D4u, 0);
    PE_StoreU32(0x800BCD60u, 0);
    PE_StoreU32(0x800B699Cu, 0);
    PE_StoreU32(0x800B6A04u, 0);
    PE_StoreU32(0x800B69ECu, 0);
    PE_StoreU32(0x800B69F0u, 0);
    PE_StoreU16(0x800B6A3Cu, 0);
    PE_StoreU16(0x800B6A38u, 0);
    PE_StoreU16(0x800B69D0u, 0);
    PE_StoreU16(0x8009D21Eu, 0);
    PE_StoreU32(0x8009D2CCu, 0);
    PE_StoreU16(0x8009D220u, 0);
    PE_StoreU32(0x8009D2D0u, 0);
    PE_StoreU16(0x8009D2A2u, 0);
    PE_StoreU32(0x800BCD6Cu, 0);
    PE_StoreU32(0x800B69B4u, 0);
    PE_StoreU32(0x800BCD70u, 0);
    PE_StoreU32(0x800B69B8u, 0);
    PE_StoreU32(0x800BCD74u, 0);
    PE_StoreU32(0x800B69BCu, 0);
    PE_StoreU32(0x800B6A24u, 0);
    PE_StoreU32(0x800B6A20u, 0);
    PE_StoreU32(0x800B6A1Cu, 0);
    PE_StoreU32(0x800B8628u, 0);
    PE_StoreU16(0x800B69E0u, 0);
    PE_StoreU16(0x800B69DEu, 0);
    PE_StoreU16(0x800B69DCu, 0);
    PE_StoreU16(0x800B69E4u, 0);
    /* D_800C0D90 attribute block */
    PE_StoreU32(0x800C0D90u, 0x3FCFu);
    PE_StoreU16(0x800C0D96u, 0x3FFFu);
    PE_StoreU16(0x800C0D94u, 0x3FFFu);
    PE_StoreU16(0x800C0D98u, 0);
    PE_StoreU16(0x800C0D9Au, 0);
    PE_StoreU16(0x800C0DA2u, 0x7FFFu);
    PE_StoreU16(0x800C0DA0u, 0x7FFFu);
    PE_StoreU32(0x800C0DA4u, 0);
    PE_StoreU32(0x800C0DA8u, 1);
    PE_StoreU16(0x800C0DAEu, 0);
    PE_StoreU16(0x800C0DACu, 0);
    PE_StoreU32(0x800C0DB0u, 0);
    PE_StoreU32(0x800C0DB4u, 0);
    /* func_80085F74 — voice-attribute applier: collapsed (see header) */
    PE_StoreU32(0x8009D268u, 0);
    PE_StoreU32(0x8009D22Cu, 0);
    PE_StoreU32(0x8009D2B8u, 0);
    PE_StoreU32(0x800C0DD8u, 0);
    PE_StoreU32(0x800C0DD4u, 0);
    PE_StoreU32(0x800C0DD0u, 0);
    PE_StoreU32(0x8009D2F4u, 0);   /* = old D_8009D268 (0) */
    PE_StoreU32(0x8009D2DCu, 0);
    PE_StoreU32(0x8009D2E0u, 0);
    /* two 24-entry loops over the 0x11C-stride voice-state tables */
    for (i = 0; i < 0x18; i++) {
        s0 = 0x800B8AC0u + 0x50u + (uint32_t)i * 0x11Cu;
        PE_StoreU32(s0 - 0x18u, 0);
        PE_StoreU32(s0 + 0xA0u, 0x18u);
        PE_StoreU16(s0 + 0x04u, 0);
        PE_StoreU32(s0 + 0x00u, 0);
        /* func_800862F4(i, 0, 0, 0, 0) — voice-param setter: collapsed */
    }
    for (i = 0; i < 0x18; i++) {
        s0 = 0x800B8AC0u + 0x50u + (uint32_t)(0x18 + i) * 0x11Cu;
        PE_StoreU32(s0 - 0x18u, 0);
        PE_StoreU32(s0 + 0xA0u, 0x18u);
        PE_StoreU16(s0 + 0x04u, 0);
        PE_StoreU32(s0 + 0x00u, 0);
        /* func_800862F4(0x18 + i, 0, 0, 0, 0) — collapsed */
    }
    /* 12-entry channel block at 0x800BC03C, i = 0xC..0x17 */
    for (i = 0xC; i < 0x18; i++) {
        v1 = 0x800BC03Cu + (uint32_t)(i - 0xC) * 0x11Cu;
        PE_StoreU32(v1 - 0x04u, 0);
        PE_StoreU32(v1 + 0xB4u, (uint32_t)i);
        PE_StoreU16(v1 + 0x18u, 1);
        PE_StoreU32(v1 + 0x14u, 0);
        PE_StoreU16(v1 + 0x9Cu, 0x7F00u);
        PE_StoreU16(v1 + 0x38u, 0);
        PE_StoreU16(v1 + 0x34u, 0);
        PE_StoreU32(v1 + 0x00u, 0);
    }
    v1 = PE_LoadU32(0x8009D2C8u);
    PE_StoreU32(v1 + 0x18u, 0);
    PE_StoreU32(v1 + 0x14u, 0);
    PE_StoreU32(v1 + 0x10u, 0);
    PE_StoreU32(v1 + 0x80u, 0);
    PE_StoreU32(v1 + 0x7Cu, 0);
    PE_StoreU32(v1 + 0x78u, 0);
    PE_StoreU32(0x800BCD68u, 1);
    PE_StoreU32(0x800BCD64u, 0x66A80000u);
    PE_StoreU32(0x800BCD5Cu, 0);
    PE_StoreU32(0x800BCD58u, 0);
    PE_StoreU32(0x800BCD54u, 0);
    PE_StoreU32(v1 + 0xA8u, 0x3FFF0000u);
    PE_StoreU32(v1 + 0x40u, 0x3FFF0000u);
    PE_StoreU32(v1 + 0xACu, 0);
    PE_StoreU32(v1 + 0x44u, 0);
    PE_StoreU16(v1 + 0xC0u, 0);
    PE_StoreU16(v1 + 0x58u, 0);
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x80u);
    /* func_8008CB54(4) — streaming-mode command: collapsed (see header) */
    /* func_80085A64(1) — SPU voice key-on: collapsed (see header) */
}

/* ── func_8008CB08 — command-ring allocator ───────────────────────────── */
static pe_addr_t PE_Stream_RingAlloc(void)
{
    pe_addr_t e = 0x800B8628u + PE_LoadU32(0x8009D2F4u) * 0x24u;
    PE_StoreU32(0x8009D2F4u, PE_LoadU32(0x8009D2F4u) + 1u);
    return e;
}

/* ── func_8008CBA8 — streaming command dispatcher ─────────────────────── */
int func_8008CBA8(void)
{
    uint32_t cmd = PE_LoadU32(0x800BCD80u);
    uint32_t a84 = PE_LoadU32(0x800BCD84u);
    uint32_t a88 = PE_LoadU32(0x800BCD88u);
    uint32_t a8C = PE_LoadU32(0x800BCD8Cu);
    uint32_t a90 = PE_LoadU32(0x800BCD90u);
    int s1 = 0;
    pe_addr_t e;

    PE_StoreU32(0x8009D268u, 1);
    switch (cmd) {
    case 0x98:
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Au);
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Cu);
        break;
    case 0x99:
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Bu);
        e = PE_Stream_RingAlloc(); PE_StoreU32(e, 0x9Du);
        break;
    case 0xD8:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD0u);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD4u);
        break;
    case 0xD9:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD1u); PE_StoreU32(e + 0x8u, a88);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD5u); PE_StoreU32(e + 0x8u, a88);
        break;
    case 0xDA:
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD2u);
        PE_StoreU32(e + 0x8u, a88); PE_StoreU32(e + 0xCu, a8C);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84); PE_StoreU32(e, 0xD6u);
        PE_StoreU32(e + 0x8u, a88); PE_StoreU32(e + 0xCu, a8C);
        break;
    case 0x24: {
        uint32_t old = PE_LoadU32(0x8009CDF0u);
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84);
        PE_StoreU32(e + 0x8u, a88);
        PE_StoreU32(e + 0xCu, a8C);
        s1 = (int)old;
        PE_StoreU32(e + 0x14u, old);
        PE_StoreU32(e + 0x10u, a90);
        PE_StoreU32(e, cmd);
        PE_StoreU32(0x8009CDF0u, ((old + 1u) & 0x1FFu) + 0x400u);
        break;
    }
    case 0x10:
    case 0x12:
    case 0x19:
        /* Streaming read path (func_80085084): not exercised at boot.
         * Trap loudly rather than fake a result. */
        Stub_Record("func_8008CBA8(read-path)", "UNSUPPORTED");
        fprintf(stderr,
                "FATAL: func_8008CBA8 streaming read path (cmd 0x%02X) reached\n",
                (unsigned)cmd);
        abort();
    default:                        /* includes 0xF0 / 0xF1 */
        e = PE_Stream_RingAlloc();
        PE_StoreU32(e + 0x4u, a84);
        PE_StoreU32(e + 0x8u, a88);
        PE_StoreU32(e + 0xCu, a8C);
        PE_StoreU32(e + 0x10u, a90);
        PE_StoreU32(e, cmd);
        break;
    }
    PE_StoreU32(0x8009D268u, 0);
    return s1;
}

/* ── Command issuers ──────────────────────────────────────────────────── */

void func_80086FF8(void)
{
    PE_StoreU32(0x800BCD80u, 0xF0u);
    func_8008CBA8();
}

void func_80087024(void)
{
    PE_StoreU32(0x800BCD80u, 0xF1u);
    func_8008CBA8();
}

void func_8008682C(int a)
{
    uint32_t v = (a == 1) ? 0x9Au : (a == 2) ? 0x9Cu : 0x98u;
    PE_StoreU32(0x800BCD80u, v);
    func_8008CBA8();
}

/* ── func_80085644 — streaming bring-up ───────────────────────────────── */

void func_80085644(void)
{
    func_8007D15C();                    /* SPU IRQ event (guard: no-op after SsInit) */

    /* func_80085A04(4, &D_800B6958) — stream transfer config */
    PE_StoreU32(0x800B6958u, 0x40001010u);
    PE_StoreU32(0x8009B464u, 0x800B6958u);
    PE_StoreU32(0x8009B460u, 0);
    PE_StoreU32(0x8009B45Cu, 4);
    PE_StoreU32(0x800B6958u + 4u, (0x10000u << PE_LoadU32(0x8009B424u)) - 0x1010u);

    /* func_80085F14(0) — mode 0 -> both words 0 */
    PE_StoreU32(0x8009B38Cu, 0);
    PE_StoreU32(0x8009B418u, 0);

    /* func_80085EB4(0x1010): (0x1010-0x1010) <= 0x7EFE8 -> alloc path.
     * func_8007DB24(-1) SPU heap query: host model top = 0x1010. */
    PE_StoreU16(0x8009B414u, 0x1010u);

    /* func_800850F4(&D_8009B7FC, 0x20): */
    PE_StoreU32(0x8009D24Cu, 1);            /* func_800850C0 */
    PE_StoreU32(0x8009B434u, 0x80085098u);  /* func_80085F44(callback) */
    /* func_80085E54: func_8007D9F8 SPU DMA upload — collapsed; completion
     * applied synchronously via the func_80085098 callback effects: */
    PE_StoreU32(0x8009B434u, 0);            /* func_80085F44(0) */
    PE_StoreU32(0x8009D24Cu, 0);
    /* func_80085174 — spin while D_8009D24C == 1 (already complete) */
    while (PE_LoadU32(0x8009D24Cu) == 1) { }

    PE_Stream_StateInit();                  /* func_80085290 */

    /* func_80085C44(0) — SPU control-register wait: collapsed hardware */

    /* func_80085D84(0) — transfer-mode setter, no change at boot */
    if (PE_LoadU32(0x8009B438u) != 0) {
        PE_StoreU32(0x8009B438u, 0);
        /* func_80085DC4 — SPU transfer-mode sequence: collapsed */
    }

    /* func_80085814/func_800858E8 DMA acquire/release retry loops:
     * host-synchronous, succeed on first poll (see header) */

    /* OpenEvent retry loop: real host event handle, != -1 immediately */
    PE_StoreU32(0x8009CDE0u,
                (uint32_t)PE_Event_Open(0xF2000002u, 2, 0x1000, 0x8008E23Cu));
    /* EnableEvent retry loop: succeeds immediately */
    PE_Event_Enable((int)PE_LoadU32(0x8009CDE0u));
}
