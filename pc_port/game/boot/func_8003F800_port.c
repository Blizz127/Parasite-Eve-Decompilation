/*
 * PE-SAVEWRITE — func_8003F800 (hand-translated, nonmatching).
 *
 * Authority: retail Disc1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b,
 * asm/disc1/2F174.s 0x8003F800..0x8003FBD8 (0x3D8, 246w).  No jal — pure data
 * movement.
 *
 * Appends the save record's extra fields to the block cursor D_800A0ED0:
 *   0x800 bytes from D_800A77F0, then a 4-word status prefix, eight status
 *   bytes, 0x70 bytes from D_800B8A20, 0x18 from D_800B0CB0 and 8 from
 *   D_8009D1B0, advancing the cursor 0x8A8 in total.
 *
 * The retail unaligned lwl/lwr copies are plain byte copies.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_SAVE_CURSOR 0x800A0ED0u

static void pe_st3_memcpy(pe_addr_t dst, pe_addr_t src, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + i));
}

void func_8003F800(void)
{
    pe_addr_t c = PE_LoadU32(GA_SAVE_CURSOR);

    pe_st3_memcpy(c, 0x800A77F0u, 0x800u);
    c += 0x800u;

    PE_StoreU32(c + 0x00u, PE_LoadU32(0x8009D2E8u));
    PE_StoreU32(c + 0x04u, PE_LoadU32(0x8009D280u));
    PE_StoreU32(c + 0x08u, PE_LoadU32(0x8009D1A0u));
    PE_StoreU32(c + 0x0Cu, PE_LoadU32(0x800B0CDCu));

    PE_StoreU8(c + 0x10u, PE_LoadU8(0x800B0CE0u));
    PE_StoreU8(c + 0x11u, PE_LoadU8(0x800B0CE1u));
    PE_StoreU8(c + 0x12u, PE_LoadU8(0x800B0CE2u));
    PE_StoreU8(c + 0x13u, PE_LoadU8(0x800B0CE3u));
    PE_StoreU8(c + 0x14u, PE_LoadU8(0x800B0CE4u));
    PE_StoreU8(c + 0x15u, PE_LoadU8(0x800B0CE5u));
    PE_StoreU8(c + 0x16u, PE_LoadU8(0x800B0CE6u));
    PE_StoreU8(c + 0x17u, PE_LoadU8(0x800BCFEEu));

    pe_st3_memcpy(c + 0x18u, 0x800B8A20u, 0x70u);
    pe_st3_memcpy(c + 0x88u, 0x800B0CB0u, 0x18u);
    pe_st3_memcpy(c + 0xA0u, 0x8009D1B0u, 8u);

    PE_StoreU32(GA_SAVE_CURSOR, c + 0xA8u);
}
