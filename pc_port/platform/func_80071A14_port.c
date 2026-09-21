/*
 * func_80071A14 — BIOS A(19h) strcpy trampoline.
 *
 * Raw body: 3 words / 0xC, exe 0x80071A14..0x80071A1F, file 0x62214.
 *   addiu $t2, $zero, 0x00A0   → BIOS A-function table vector
 *   jr $t2
 *   addiu $t1, $zero, 0x0019   → A(19h) = strcpy(dst, src)
 *
 * psx-spx "BIOS String Functions": A(19h) strcpy(dst,src).  Copies including
 * the terminating NUL and returns the incoming dst.
 */
#include "psx_compat.h"

pe_addr_t func_80071A14(pe_addr_t dst, pe_addr_t src)
{
    pe_addr_t d = dst;

    for (;;) {
        uint8_t c = PE_LoadU8(src);
        src++;
        PE_StoreU8(d, c);
        d++;
        if (c == 0u)
            break;
    }
    return dst;
}
