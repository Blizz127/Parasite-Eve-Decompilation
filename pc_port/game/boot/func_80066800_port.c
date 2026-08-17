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

/*
 * PE-BTL64 — func_800661EC camera-request leaf.
 *
 * 31 words 0x800661EC..0x80066268, SHA-256
 * 095ed0474107c1b7124d63d5dc36c7fbc97f82ac409ee06746741734bc0bba28.
 * Zero jal. Callers 17C54 (0x03, a3=0) and 17C8C (0x46, a3=8).
 * If BCF88 bit 0x40 is clear, return -19 with no stores.
 * Else BCFA0=1, BCF9C/9E/A2=a0/a1/a2, BCF98=*BCF8C,
 * BCF88 = (flags & 0xFFF0) | (a3==8 ? 1 : 9), return 0.
 */
int func_800661EC(int a0, int a1, unsigned int a2, unsigned int a3)
{
    uint32_t flags;
    uint32_t low;

    flags = PE_LoadU32(GA_D_800BCF88);
    if ((flags & 0x40u) == 0u)
        return -19;
    PE_StoreU16(0x800BCFA0u, 1u);
    PE_StoreU16(0x800BCF9Cu, (uint16_t)a0);
    PE_StoreU16(0x800BCF9Eu, (uint16_t)a1);
    PE_StoreU16(0x800BCFA2u, (uint16_t)a2);
    PE_StoreU32(0x800BCF98u, PE_LoadU32(0x800BCF8Cu));
    low = (a3 == 8u) ? 1u : 9u;
    PE_StoreU32(GA_D_800BCF88, (flags & 0xFFF0u) | low);
    return 0;
}
