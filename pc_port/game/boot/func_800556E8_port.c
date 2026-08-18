/*
 * PE-BTL121 — menu index table 0x800A1D9C.
 *
 * 55610: for i in 0..19, if C0E24 bit i is set, pack i into the
 * table and store the packed count at gp+0x2D0.
 * 556E8(a0): if 0 <= a0 < count, return table[a0] (signed half).
 *
 * 46C20 sw 556E8() to gp+0x244; 57B70 uses that as 512AC(1)
 * index. Bit 19 packed → value 19 → D010=406.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800C0E24 0x800C0E24u
#define GA_T_800A1D9C 0x800A1D9Cu
#define GA_D_8009D040 0x8009D040u /* gp+0x2D0 count */
#define GA_D_8009D068 0x8009D068u /* gp+0x2F8 */

void func_80055610(void)
{
    uint32_t bits;
    pe_addr_t dst;
    int i;
    int packed;

    bits = PE_LoadU32(GA_D_800C0E24);
    dst = GA_T_800A1D9C;
    packed = 0;
    for (i = 0; i < 20; i++) {
        if ((bits & 1u) != 0u) {
            PE_StoreU16(dst, (uint16_t)i);
            dst += 2u;
            packed++;
        }
        bits >>= 1;
    }
    PE_StoreU32(GA_D_8009D068, 0u);
    PE_StoreU32(GA_D_8009D040, (uint32_t)packed);
}

int func_800556E8(int index)
{
    int32_t count;

    if (index < 0)
        return 0;
    count = (int32_t)PE_LoadU32(GA_D_8009D040);
    if (index >= count)
        return 0;
    return (int)(int16_t)PE_LoadU16(GA_T_800A1D9C + (uint32_t)index * 2u);
}
