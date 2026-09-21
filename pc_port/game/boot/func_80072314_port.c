/* BIOS A-function string helpers used by the PsyQ formatter (func_80071A84)
 * for the "%s" conversion with and without an explicit precision.
 *
 * Retail func_80072314 (asm/disc1/621E4.s @0x80072314, 12 bytes) is a pure
 * BIOS A0 thunk:
 *     addiu $t2,$zero,0xA0 ; jr $t2 ; addiu $t1,$zero,0x1B
 *   = A(1Bh) strlen(src) -> length in $v0 (psx-spx "BIOS String Functions").
 * Retail func_80072324 (@0x80072324, 12 bytes) is the same thunk with
 * 0x2E:
 *   = A(2Eh) memchr(src,scanbyte,len) -> pointer to the first matching byte
 *     inside the first len bytes, or 0 (psx-spx "BIOS Memory Fill/Copy/
 *     Compare").  The formatter only ever calls it as memchr(src,0,len) to
 *     bound a "%N.Ns" copy by the terminator.
 *
 * Both thunks are SDK/BIOS host implementations (classification 2 in the
 * pc_port scheme), the same treatment the rest of pc_port/platform gives the
 * BIOS A/B/C vectors.  There is no decompiled original body to reproduce: the
 * retail code is a jump into the BIOS vector, and psx-spx fixes the
 * observable contract implemented here.
 *
 * Guest addresses go through PE_LoadU8 so KSEG0/KSEG1/physical aliases behave
 * as they do for the other ported SDK leaves. */
#include "psx_compat.h"
#include "pe_port_compat.h"

uint32_t func_80072314(pe_addr_t src)
{
    uint32_t length = 0u;
    while (PE_LoadU8(src + length) != 0u)
        length++;
    return length;
}

pe_addr_t func_80072324(pe_addr_t src, int32_t scanbyte, uint32_t len)
{
    uint32_t i;
    for (i = 0u; i < len; i++) {
        if (PE_LoadU8(src + i) == (uint8_t)scanbyte)
            return src + i;
    }
    return 0u;
}
