/*
 * PE-CH2 — func_80066800: opcode 0x82 52-byte view-record apply.
 *
 * Complete retail body (99 words / 0x18C, exe 0x80066800–0x8006698C,
 * file offset 0x57000).  Callers are opcode handler func_80018E58
 * (jal 0x80018E6C) and func_800677FC (jal 0x80067834).
 *
 * Record = *(D_800B1624) + offset_at_+0x1C + index*52.
 * Retail publishes H through *D_800BCFA8 and SetGeomScreen, copies the
 * nine rotation halfwords and three translation words through
 * *D_800BCFA4, records the byte index, and sets bit 0x80 in D_800BCF88.
 * Bytes at record+0x20 and beyond are not read.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B1624 0x800B1624u
#define GA_D_800BCF88 0x800BCF88u
#define GA_D_800BCFA4 0x800BCFA4u
#define GA_D_800BCFA8 0x800BCFA8u
#define GA_D_800BCFFD 0x800BCFFDu

int func_80066800(unsigned int index)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t record = container + PE_LoadU32(container + 0x1Cu)
                     + index * 52u;
    pe_addr_t h_dest = PE_LoadU32(GA_D_800BCFA8);
    uint16_t h;
    unsigned int i;

    h = PE_LoadU16(record);
    PE_StoreU32(h_dest, h);
    func_80079024((int)h);

    for (i = 0; i < 9u; i++) {
        pe_addr_t matrix = PE_LoadU32(GA_D_800BCFA4);
        PE_StoreU16(matrix + i * 2u, PE_LoadU16(record + 2u + i * 2u));
    }
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x14u,
                PE_LoadU32(record + 0x14u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x18u,
                PE_LoadU32(record + 0x18u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x1Cu,
                PE_LoadU32(record + 0x1Cu));

    PE_StoreU8(GA_D_800BCFFD, (uint8_t)index);
    PE_StoreU32(GA_D_800BCF88, PE_LoadU32(GA_D_800BCF88) | 0x80u);
    return 0;
}
