/*
 * PE-BTL6 — func_8003C5D8 (24 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x8003C5D8..0x8003C638. Live EE=13 site is
 * jal 3C5D8(dest, 50) at 0x8003D764; delay sb -1 at dest+0x8C is
 * the caller's store, not this leaf. a1==0 forces divisor 1.
 * Does not jal 794C4 and does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003C5D8(pe_addr_t dest, int a1)
{
    int scale;
    int quot;

    if (dest == 0u)
        return;

    scale = a1;
    if ((a1 << 16) == 0)
        scale = 1;
    quot = 128 / (int)(int16_t)scale;
    PE_StoreU8(dest + 0x8Du, (uint8_t)scale);
    PE_StoreU8(dest + 0x8Eu, (uint8_t)quot);
    PE_StoreU8(dest + 0x8Fu, (uint8_t)quot);
    PE_StoreU8(dest + 0x93u, (uint8_t)quot);
}

/*
 * PE-BTL120 — 3C818 dest fade (178w 0x8003C818..0x8003CAE0)
 * and 3AF14 dest tick (140w 0x8003AF14..0x8003B144).
 *
 * Aya+0x1B4 is an embedded dest, so dest+0x9E IS Aya+0x252
 * and dest+0x9C IS Aya+0x250. 24A3C case 2's sb 1 at +0x252
 * is the dest busy byte. The EXE has no sb $0,594(actor);
 * the zero store is sb $0,158(dest) at 3C87C when +0x8C==1.
 *
 * 3C818 +0x8C machine:
 *   0  → sb -1, return (no decrement)
 *   1  → dest+0x9E=0, then +0x8C--
 *  <0  → copy +0x8D → +0x8C, then --
 *  >=2 → 3CCB0/color deferred, then --
 * 3B97C / 3BCE0 / 3CCB0 / 3CEF8 stay deferred.
 *
 * 3AF14: dest+0==0 or lh +0xBA==0 return. dest+0x9C&2
 * jals 3C818 (case 0/5 +0x250|=2). 3C2E0 / 3B144 / 3C638
 * stay deferred.
 */
int func_8003C818(pe_addr_t dest)
{
    int8_t phase;

    if (dest == 0u)
        return 0;
    if (PE_LoadU32(dest) == 0u)
        return 0;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return 0;

    phase = (int8_t)PE_LoadU8(dest + 0x8Cu);
    if (phase == 0) {
        PE_StoreU8(dest + 0x8Cu, 0xFFu);
        return 0;
    }
    if (phase == 1)
        PE_StoreU8(dest + 0x9Eu, 0u);
    else if (phase < 0)
        PE_StoreU8(dest + 0x8Cu, PE_LoadU8(dest + 0x8Du));
    PE_StoreU8(dest + 0x8Cu, (uint8_t)(PE_LoadU8(dest + 0x8Cu) - 1u));
    return 0;
}

int func_8003AF14(pe_addr_t dest, pe_addr_t scratch)
{
    uint16_t flags;

    (void)scratch;
    if (dest == 0u)
        return 0;
    if (PE_LoadU32(dest) == 0u)
        return 0;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return 0;

    flags = PE_LoadU16(dest + 0x9Cu);
    /* 3C2E0 (+0x9C&0x10) and 3B144 (+0x9E==1 draw) deferred. */
    if ((flags & 2u) != 0u)
        return func_8003C818(dest);
    /* 3C638 (+0x9C&4 without bit 1) deferred. */
    return 0;
}
