/*
 * Phase 6E-B54I — func_80077C84: draw-mode (DR_MODE) word builder
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body including its return delay slot (11 words, verified
 * against the SHA-1-exact retail executable SLUS_006.62 / SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b; file 0x68484).  Decoded from
 * build/disc1.candidate.exe @ file 0x68484:
 *
 *   0x80077C84: 0x24020001  addiu v0, zero, 1
 *   0x80077C88: 0xA0820003  sb    v0, 3(a0)       ; p[3] = 1   (length)
 *   0x80077C8C: 0x10C00002  beq   a2, zero, 0x80077C98
 *   0x80077C90: 0x3C03E100  lui   v1, 0xE100      ; delay slot (always)
 *   0x80077C94: 0x34630200  ori   v1, v1, 0x200   ; only if a2 != 0
 *   0x80077C98: 0x10A00002  beq   a1, zero, 0x80077CA4
 *   0x80077C9C: 0x30E209FF  andi  v0, a3, 0x9FF   ; delay slot (always)
 *   0x80077CA0: 0x34420400  ori   v0, v0, 0x400   ; only if a1 != 0
 *   0x80077CA4: 0x00621025  or    v0, v1, v0
 *   0x80077CA8: 0x03E00008  jr    ra
 *   0x80077CAC: 0xAC820004  sw    v0, 4(a0)       ; delay slot: p[4..7]
 *
 * The delay-slot store at 0x80077CAC is INSIDE this function's footprint
 * (verified word in the SHA-exact image); the next function begins at
 * 0x80077CB0 with an alignment nop.
 *
 * Semantics: p[3] = 1, then the GP0(E1h)-shaped draw-mode word
 *   word = (0xE1000000 | (a2 ? 0x200 : 0))
 *        | ((a3 & 0x9FF) | (a1 ? 0x400 : 0))
 * is stored as a 32-bit word at p+4 and returned in v0.
 *
 * Callers: func_800370DC and func_80037140 (@0x800370F8 / 0x8003715C) —
 * both pass (p, a1=0, a2=1, a3=tpage), giving 0xE1000200 | (tpage & 0x9FF).
 * Neither wrapper consumes the return value.
 *
 * ABI: uint32_t func_80077C84(pe_addr_t p, uint32_t a1, uint32_t a2,
 * uint32_t a3).  Writes byte p+3 and the word at p+4; nothing else.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

uint32_t func_80077C84(pe_addr_t p, uint32_t a1, uint32_t a2, uint32_t a3)
{
    PE_StoreU8(p + 3, 1);

    uint32_t v1 = 0xE1000000u;
    if (a2 != 0)
        v1 |= 0x0200u;

    uint32_t v0 = a3 & 0x09FFu;
    if (a1 != 0)
        v0 |= 0x0400u;

    v0 |= v1;
    PE_StoreU32(p + 4, v0);
    return v0;
}
