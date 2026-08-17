/*
 * PE-BTL6 — func_8003DFD8 live copy leaf (51 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x8003DFD8..0x8003E0A4 (first jr). The 73-word find_fn_end
 * window walks into two later leaves. Live 3D834 a1==0 site is
 * jal 3DFD8(0x800B1638, dest+0x34, 1). Copies count 32-byte records:
 * halfwords +0..+16, words +0x14/+0x18/+0x1C; +0x12 is not touched.
 * blez count returns. Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003DFD8(pe_addr_t src, pe_addr_t dst, int count)
{
    int i;

    count = (int)(int16_t)count;
    if (count <= 0 || src == 0u || dst == 0u)
        return;

    for (i = 0; i < count; i++) {
        PE_StoreU16(dst + 0x00u, PE_LoadU16(src + 0x00u));
        PE_StoreU16(dst + 0x02u, PE_LoadU16(src + 0x02u));
        PE_StoreU16(dst + 0x04u, PE_LoadU16(src + 0x04u));
        PE_StoreU16(dst + 0x06u, PE_LoadU16(src + 0x06u));
        PE_StoreU16(dst + 0x08u, PE_LoadU16(src + 0x08u));
        PE_StoreU16(dst + 0x0Au, PE_LoadU16(src + 0x0Au));
        PE_StoreU16(dst + 0x0Cu, PE_LoadU16(src + 0x0Cu));
        PE_StoreU16(dst + 0x0Eu, PE_LoadU16(src + 0x0Eu));
        PE_StoreU16(dst + 0x10u, PE_LoadU16(src + 0x10u));
        PE_StoreU32(dst + 0x14u, PE_LoadU32(src + 0x14u));
        PE_StoreU32(dst + 0x18u, PE_LoadU32(src + 0x18u));
        PE_StoreU32(dst + 0x1Cu, PE_LoadU32(src + 0x1Cu));
        src += 32u;
        dst += 32u;
    }
}
