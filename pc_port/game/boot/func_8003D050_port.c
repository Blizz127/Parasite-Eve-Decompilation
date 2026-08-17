/*
 * PE-BTL6 — func_8003D050 named cuts.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Retail 505 words 0x8003D050..0x8003D834. Exclusive named cuts:
 *   prefix     0x8003D078..0x8003D0D4  dest+0/4/8/C/10, +0x54, +0xBA
 *   ptr14      0x8003D36C..0x8003D394  dest+0x14/18/1C/20
 *   post-skip  0x8003D5D8..0x8003D750  after blez-skip of jal 3D94C
 *   epilogue   0x8003D76C..0x8003D834  after jal 3C5D8 (no 794C4)
 *
 * Live EE=13 stack word at 0x54(sp) is 0, so jal 3D94C is skipped.
 * jal 794C4 at 0x8003D750 is live and not this file. Packet-fill
 * loops 0x8003D0F0..0x8003D368 are stream side effects; dest+0x14
 * math matches the t8==0 skip at 0x8003D338. Does not andi 0xFC.
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

void func_8003D050_ptr14_cut(pe_addr_t dest, pe_addr_t obj)
{
    unsigned int n;
    pe_addr_t cursor;

    if (dest == 0u || obj == 0u)
        return;

    n = (unsigned int)PE_LoadU16(obj + 0x08u)
      + (unsigned int)PE_LoadU16(obj + 0x0Au)
      + (unsigned int)PE_LoadU16(obj + 0x0Cu)
      + (unsigned int)PE_LoadU16(obj + 0x0Eu);
    cursor = PE_LoadU32(dest + 0x10u) + n * 12u;
    PE_StoreU32(dest + 0x14u, cursor);
    cursor += 16u;
    PE_StoreU32(dest + 0x18u, cursor);
    cursor += (pe_addr_t)PE_LoadU8(obj + 2u) * 16u;
    PE_StoreU32(dest + 0x1Cu, cursor);
    cursor += 8u;
    PE_StoreU32(dest + 0x20u, cursor);
}

int func_8003D050_post_3d94c_skip_cut(pe_addr_t dest, pe_addr_t stream)
{
    pe_addr_t obj;
    pe_addr_t cursor;
    unsigned int count;
    unsigned int i;
    int skipped;

    if (dest == 0u)
        return 0;

    obj = PE_LoadU32(dest + 0x00u);
    PE_StoreU16(dest + 0x70u, PE_LoadU16(PE_LoadU32(dest + 0x14u) + 6u));
    PE_StoreU16(dest + 0x72u, PE_LoadU16(PE_LoadU32(dest + 0x1Cu) + 6u));
    PE_StoreU32(dest + 0x80u, stream);
    cursor = stream;
    skipped = 0;
    count = (obj != 0u) ? PE_LoadU8(obj + 2u) : 0u;

    if (count != 0u) {
        pe_addr_t rec = cursor + 6u;
        for (i = 0; i < count; i++) {
            pe_addr_t entry = PE_LoadU32(dest + 0x04u) + i * 12u;
            if (PE_LoadU8(entry + 4u) != 1u) {
                skipped++;
                continue;
            }
            {
                pe_addr_t slot = PE_LoadU32(dest + 0x18u) + i * 16u;
                if ((int16_t)PE_LoadU16(slot + 0x0Eu) < 0)
                    continue;
                PE_StoreU16(rec + 2u, PE_LoadU16(slot + 6u));
                cursor += 12u;
                PE_StoreU16(rec + 0u, (uint16_t)i);
                PE_StoreU16(rec + 4u, PE_LoadU16(slot + 0x0Eu));
                rec += 12u;
            }
        }
    }
    PE_StoreU32(dest + 0x84u, cursor);

    if (count != 0u) {
        for (i = 0; i < count; i++) {
            PE_StoreU16(cursor + 0x00u, 4096u);
            PE_StoreU16(cursor + 0x02u, 0u);
            PE_StoreU16(cursor + 0x04u, 0u);
            PE_StoreU16(cursor + 0x06u, 0u);
            PE_StoreU16(cursor + 0x08u, 4096u);
            PE_StoreU16(cursor + 0x0Au, 0u);
            PE_StoreU16(cursor + 0x0Cu, 0u);
            PE_StoreU16(cursor + 0x0Eu, 0u);
            PE_StoreU16(cursor + 0x10u, 4096u);
            PE_StoreU32(cursor + 0x14u, 0u);
            PE_StoreU32(cursor + 0x18u, 0u);
            PE_StoreU32(cursor + 0x1Cu, 0u);
            cursor += 32u;
        }
    }

    if (obj != 0u)
        PE_StoreU16(obj + 0x14u, 0u);
    PE_StoreU16(dest + 0x2Cu, 0u);
    PE_StoreU16(dest + 0x2Eu, 0u);
    PE_StoreU16(dest + 0x30u, 0u);
    PE_StoreU16(dest + 0x32u, 1u);
    PE_StoreU16(dest + 0x2Cu, 0u);
    PE_StoreU16(dest + 0x2Eu, 0u);
    PE_StoreU16(dest + 0x30u, 0u);
    return skipped;
}

void func_8003D050_epilogue_cut(pe_addr_t dest, int skipped)
{
    pe_addr_t obj;
    unsigned int half;
    int i;

    if (dest == 0u)
        return;

    PE_StoreU8(dest + 0x8Cu, (uint8_t)-1);
    PE_StoreU8(dest + 0x90u, 0x80u);
    PE_StoreU8(dest + 0x91u, 0x80u);
    PE_StoreU8(dest + 0x92u, 0x80u);
    PE_StoreU8(dest + 0x9Eu, 1u);
    PE_StoreU16(dest + 0x9Cu, 0u);
    PE_StoreU16(dest + 0x28u, 0u);
    PE_StoreU32(dest + 0x24u, 0u);
    PE_StoreU16(dest + 0x2Au, 0u);
    PE_StoreU8(dest + 0x9Fu, (uint8_t)D_8009CDDC);
    obj = PE_LoadU32(dest + 0x00u);
    if (obj != 0u) {
        half = PE_LoadU16(obj + 6u);
        PE_StoreU16(obj + 0x1Au, (uint16_t)((int)half - skipped));
    }
    half = PE_LoadU16(dest + 0x72u);
    PE_StoreU16(dest + 0x6Eu,
                (uint16_t)(PE_LoadU16(PE_LoadU32(dest + 0x1Cu) + 2u)
                           + ((int16_t)half >> 4)));
    for (i = 0; i < 2; i++) {
        pe_addr_t slot = dest + (pe_addr_t)(i * 8);
        PE_StoreU8(slot + 0xA6u, 0u);
        PE_StoreU8(slot + 0xA7u, 0u);
    }
    PE_StoreU32(dest + 0xB0u, 0u);
}
