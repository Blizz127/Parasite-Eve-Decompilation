/*
 * Phase 6E-A — libgte geometry-engine control state.
 *
 * func_80077F7C = PsyQ InitGeom (asm/disc1/68478.s, handwritten asm).
 * Classification: 2/3 (SDK host implementation with deterministic constants).
 *
 * Collapsed retail effects (hardware-only, no guest-RAM state):
 *   - func_8007A134: GTE kernel patch — EnterCriticalSection, BIOS B(56h),
 *     compare/patch 6 kernel words, FlushCache, ExitCriticalSection.
 *     Host kernel needs no GTE workaround patch: no-op.
 *   - SR |= 0x40000000 (cop2 enable): host has no coprocessor SR: no-op.
 *
 * Reproduced retail effects: the seven ctc2 control-register writes.
 * cop2 registers have no guest-RAM address, so they live in g_pe_gte.
 */
#include "pe_sdk.h"

PeGteState g_pe_gte;

void func_80077F7C(void)
{
    g_pe_gte.zsf3 = 0x155;      /* ctc2 $29 */
    g_pe_gte.zsf4 = 0x100;      /* ctc2 $30 */
    g_pe_gte.h    = 0x3E8;      /* ctc2 $26 */
    g_pe_gte.dqa  = -0x1062;    /* ctc2 $27 (0xFFFFEF9E) */
    g_pe_gte.dqb  = 0x1400000;  /* ctc2 $28 */
    g_pe_gte.ofx  = 0;          /* ctc2 $24 */
    g_pe_gte.ofy  = 0;          /* ctc2 $25 */
}

/* SetGeomOffset (asm leaf): OFX = a << 16, OFY = b << 16 */
void func_80079004(int a, int b)
{
    g_pe_gte.ofx = a << 16;
    g_pe_gte.ofy = b << 16;
}

/* SetGeomScreen (asm leaf): H = a */
void func_80079024(int a)
{
    g_pe_gte.h = a;
}

/* Phase 6E-B4 — exact 32-bit GTE LZCS/LZCR arithmetic (leading-sign-bit
 * count), factored out of the func_8003EAC8 translation.  No
 * __builtin_clz(0) undefined behavior, no floating point, no g_pe_gte
 * state: LZCS/LZCR are data registers with no caller-observable lifetime
 * past the instruction sequence that uses them. */
uint32_t PE_GTE_LZCR(uint32_t v)
{
    uint32_t sign = v & 0x80000000u;
    uint32_t n = 0;
    while (n < 32u && (v & 0x80000000u) == sign) {
        n++;
        v <<= 1;
    }
    return n;
}
