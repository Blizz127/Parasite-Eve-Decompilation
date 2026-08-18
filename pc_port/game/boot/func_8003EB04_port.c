/*
 * PE-BTL56 — func_8003EB04 pad-edge named cut (translated
 * retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 348 words 0x8003EB04..0x8003F074, SHA-256
 * c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b.
 * Sole jal 3F3C4 @ 0x8003F40C.
 *
 * This cut is the digital rebuild + newly-pressed edge only:
 *   previous = old D26C
 *   raw = lhu 0x800BE9A2 (active-low)
 *   swap 0x2000/0x4000 on the inverted word
 *   D26C |= (1<<i) when (mapped & A76F0[i])
 *   D1F4 = (held ^ previous) & held
 *   D1E4 = (held ^ previous) & previous
 *
 * 825C0 PadGetState, analog, and the 3EB38 init (D1F4=4) are
 * not this cut. A76F0 must already be filled by 3E974.
 * Message 0x100 is processed bit 8 = raw Cross after the swap.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BE9A2 0x800BE9A2u
#define GA_D_800A76F0 0x800A76F0u
#define GA_D_8009D26C 0x8009D26Cu
#define GA_D_8009D238 0x8009D238u
#define GA_D_8009D1F4 0x8009D1F4u
#define GA_D_8009D1E4 0x8009D1E4u
#define GA_D_800B0CD8 0x800B0CD8u

void func_8003EB04(void)
{
    uint32_t raw;
    uint32_t inv;
    uint32_t mapped;
    uint32_t held;
    uint32_t prev;
    uint32_t bits;
    unsigned int i;

    raw = PE_LoadU16(GA_D_800BE9A2);
    inv = (~raw) & 0xFFFFu;
    mapped = inv & 0x9FFFu;
    if ((inv & 0x2000u) != 0u)
        mapped |= 0x4000u;
    if ((inv & 0x4000u) != 0u)
        mapped |= 0x2000u;
    prev = PE_LoadU32(GA_D_8009D26C);
    PE_StoreU32(GA_D_8009D238, prev);
    held = 0u;
    for (i = 0; i < 32u; i++) {
        if ((mapped & PE_LoadU32(GA_D_800A76F0 + i * 4u)) != 0u)
            held |= (1u << i);
    }
    bits = PE_LoadU32(GA_D_800B0CD8);
    if ((bits & 0x400u) != 0u)
        held &= 0x40FFDC7Fu;
    if ((bits & 0x200u) != 0u)
        held &= 0x60FFDB04u;
    PE_StoreU32(GA_D_8009D26C, held);
    PE_StoreU32(GA_D_8009D1F4, (held ^ prev) & held);
    PE_StoreU32(GA_D_8009D1E4, (held ^ prev) & prev);
}
