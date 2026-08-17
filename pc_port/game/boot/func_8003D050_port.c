/*
 * PE-BTL6 — func_8003D050 pointer-install prefix.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_ee13_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Retail 505 words 0x8003D050..0x8003D834. This cut is the proven
 * header walk 0x8003D078..0x8003D0D0: dest+0/4/8/C/10 pointer
 * ladder, dest+0x54 = a2, dest+0xBA = stack word 6 (EE=13 stores 1).
 * Does not jal 3D94C/794C4/3C5D8 and does not reach andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003D050_prefix_cut(pe_addr_t dest, pe_addr_t obj, pe_addr_t stream,
                              unsigned int stack_ba)
{
    unsigned int count;
    unsigned int half;
    pe_addr_t cursor;

    if (dest == 0u || obj == 0u)
        return;

    PE_StoreU32(dest + 0x00u, obj);
    cursor = obj + 0x1Cu;
    PE_StoreU32(dest + 0x04u, cursor);
    count = PE_LoadU8(obj + 2u);
    cursor += count * 12u;
    PE_StoreU32(dest + 0x08u, cursor);
    half = PE_LoadU16(obj + 6u);
    cursor += (pe_addr_t)half << 3;
    PE_StoreU32(dest + 0x0Cu, cursor);
    PE_StoreU32(dest + 0x54u, stream);
    cursor += (pe_addr_t)half << 2;
    PE_StoreU16(dest + 0xBAu, (uint16_t)stack_ba);
    PE_StoreU32(dest + 0x10u, cursor);
}
