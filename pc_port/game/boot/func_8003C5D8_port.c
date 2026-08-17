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
