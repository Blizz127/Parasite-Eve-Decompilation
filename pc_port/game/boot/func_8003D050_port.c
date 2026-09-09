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

/* Retail 3D0F0..3D5A4: packet headers and UV payloads. The returned
 * cursor is s1 at 3D5D8; packet coordinates/colors remain for pose/draw. */
pe_addr_t func_8003D050_packets(pe_addr_t dest)
{
    static const uint8_t stride[4] = {52, 40, 36, 28};
    static const uint8_t code[4] = {0x3C, 0x34, 0x38, 0x30};
    static const uint8_t semi_type[4] = {11, 16, 21, 26};
    pe_addr_t obj = PE_LoadU32(dest);
    pe_addr_t packet = PE_LoadU32(dest + 0x54u);
    pe_addr_t geometry = PE_LoadU32(dest + 0x10u);
    pe_addr_t uv;
    unsigned int kind, i, bank, vertex;
    if (!PE_LoadU16(dest + 0xBAu))
        return packet;
    for (kind = 0; kind < 4u; kind++) {
        unsigned int count = PE_LoadU16(obj + 8u + kind * 2u);
        for (i = 0; i < count; i++, geometry += 12u) {
            uint8_t command = code[kind];
            if (PE_LoadU8(geometry + 3u) == semi_type[kind])
                command |= 2u;
            for (bank = 0; bank < 2u; bank++, packet += stride[kind]) {
                PE_StoreU8(packet + 3u, (uint8_t)(stride[kind] / 4u - 1u));
                PE_StoreU8(packet + 7u, command);
            }
        }
    }
    uv = PE_LoadU32(dest + 0x20u) + ((PE_LoadU16(obj + 0x18u) + 3u) & ~3u);
    {
        pe_addr_t cursor = PE_LoadU32(dest + 0x54u);
        for (kind = 0; kind < 2u; kind++) {
            unsigned int vertices = 4u - kind;
            unsigned int count = PE_LoadU16(obj + 8u + kind * 2u);
            for (i = 0; i < count; i++, uv += vertices * 4u) {
                for (bank = 0; bank < 2u; bank++, cursor += stride[kind]) {
                    for (vertex = 0; vertex < vertices; vertex++) {
                        PE_StoreU8(cursor + 12u + vertex * 12u, PE_LoadU8(uv + vertex * 4u));
                        PE_StoreU8(cursor + 13u + vertex * 12u, PE_LoadU8(uv + vertex * 4u + 1u));
                    }
                    PE_StoreU16(cursor + 14u, PE_LoadU16(uv + 2u));
                    PE_StoreU16(cursor + 26u, PE_LoadU16(uv + 6u));
                }
            }
        }
    }
    return packet;
}

/* Retail 3D94C..3DBE4. a3 is unused by the original leaf. */
void func_8003D94C(pe_addr_t dest, int x, int y, int unused, int palette_y)
{
    pe_addr_t packet = PE_LoadU32(dest + 0x54u);
    pe_addr_t obj = PE_LoadU32(dest);
    int sx = (int16_t)x, sy = (int16_t)y;
    int page = ((sx >> 6) - 8) * 2 + (sy >> 7) + (sy >> 8) * 30;
    uint32_t page_delta;
    uint32_t clut_delta = (uint32_t)palette_y * 64u - 0x7080u;
    unsigned int kind, i, bank, vertex;
    (void)unused;
    if (sx == 0x3C0) {
        if (PE_LoadU16(packet + 26u) == 31u)
            return;
        page_delta = (uint32_t)((int16_t)page >> 1);
    } else {
        page_delta = (uint32_t)page >> 1;
    }
    for (kind = 0; kind < 2u; kind++) {
        unsigned int count = PE_LoadU16(obj + 8u + kind * 2u);
        unsigned int stride = kind ? 40u : 52u;
        for (i = 0; i < count; i++) {
            for (bank = 0; bank < 2u; bank++, packet += stride) {
                uint32_t adjustment = page_delta;
                if (sy == 128) {
                    if (PE_LoadU8(packet + 13u) >= 128u)
                        adjustment++;
                    for (vertex = 0; vertex < 4u - kind; vertex++) {
                        pe_addr_t v = packet + 13u + vertex * 12u;
                        PE_StoreU8(v, (uint8_t)(PE_LoadU8(v) + 128u));
                    }
                }
                PE_StoreU16(packet + 26u, (uint16_t)(PE_LoadU16(packet + 26u) + adjustment));
                PE_StoreU16(packet + 14u, (uint16_t)(PE_LoadU16(packet + 14u) + clut_delta));
            }
        }
    }
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
    PE_StoreU8(dest + 0x9Fu, (uint8_t)PE_LoadU32(0x8009CDDCu));
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

/*
 * PE-BTL119 — 3D834 (68 words). a1==0 skips the clip bind and
 * jals 3A088. Both arms 3DFD8(B1638, dest+52, 1), 3B97C, two
 * 3BCE0 with PE_LoadU32(0x8009CDDCu) toggled. 39B74 is a no-op when clip=0.
 */
#define GA_B1638  0x800B1638u
#define GA_91A38  0x80091A38u
#define GA_9CDDC  0x8009CDDCu

void func_8003D834(pe_addr_t dest, pe_addr_t clip, int a2, pe_addr_t bea40)
{
    uint32_t word;
    int16_t half;

    if (dest == 0u)
        return;
    if (clip != 0u) {
        PE_StoreU32(dest + 0xB0u, clip);
        func_8003DFD8(dest + 52u, GA_B1638, 1);
        func_8003DFD8(GA_91A38, dest + 52u, 1);
        func_80039B74(dest, clip, (int)(int16_t)a2, 0);
    }
    func_8003A088_mode0_walk_cut(dest);
    func_8003DFD8(GA_B1638, dest + 52u, 1);
    func_8003B97C_lighting_cut(dest, bea40);
    half = (int16_t)PE_LoadU16(GA_9CDDC);
    func_8003BCE0(dest, 1, (int)half);
    word = PE_LoadU32(GA_9CDDC) ^ 1u;
    PE_StoreU32(GA_9CDDC, word);
    func_8003BCE0(dest, 1, (int)(int16_t)word);
    word = PE_LoadU32(GA_9CDDC) ^ 1u;
    PE_StoreU32(GA_9CDDC, word);
}
