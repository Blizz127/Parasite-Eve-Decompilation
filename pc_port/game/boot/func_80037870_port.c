/*
 * PE-BTL55 — func_80037870 message updater named cut
 * (translated retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 1064 words 0x80037870..0x80038910, SHA-256
 * 2b0eeaf2ed110c388b805b4cb3759bcf0161f1d35cc434a218d5b8f68d8c9d4d.
 * Sole live jal 3F3C4 @ 0x8003F568 when B0CD8&0x200==0.
 *
 * This cut is the proven TXT0 state machine only:
 *   state 1: copy gp+0x120 (D_8009CE90) to +0x04, scan
 *     (FF|F9) FE <id>, cursor = marker+3
 *   parse until a yielding control
 *   F9 → state 0
 *   FF + state!=2 → parser_stop (388AC promotes 1→2)
 *   FF + state==2 + (D1F4&0x100) + !(flags&0x02000000) → 0
 *   FB 07 nn pauses on +0x0D
 *
 * Glyph SPRT / 5E894 / 61C34 / 77AC4 are not this cut.
 * Host refuses to walk a non-KSEG stream (ROM would load
 * address 0). Scan length is capped at 0x10000 —
 * APPROXIMATION; ROM has no bound.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009CE90 0x8009CE90u
#define GA_D_8009D1F4 0x8009D1F4u
#define REC_STRIDE    56u
#define SCAN_CAP      0x10000u

static int pe_37870_kseg(pe_addr_t p)
{
    return p >= 0x80000000u && p < 0x80200000u;
}

static pe_addr_t pe_37870_find_marker(pe_addr_t stream, int id)
{
    unsigned int n;
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;

    if (!pe_37870_kseg(stream))
        return 0;
    for (n = 0; n < SCAN_CAP; n++) {
        if (!pe_37870_kseg(stream + 2u))
            return 0;
        b0 = PE_LoadU8(stream);
        if (b0 == 0xFFu || b0 == 0xF9u) {
            b1 = PE_LoadU8(stream + 1u);
            b2 = PE_LoadU8(stream + 2u);
            if (b1 == 0xFEu && (int)b2 == id)
                return stream + 3u;
        }
        stream++;
    }
    return 0;
}

static void pe_37870_record(pe_addr_t rec)
{
    uint8_t state;
    pe_addr_t cursor;
    pe_addr_t stream;
    int id;
    uint32_t pad;
    uint32_t flags;
    unsigned int guard;
    uint8_t byte;
    uint8_t sub;
    uint8_t nn;
    uint8_t pause;
    int stop;

    state = PE_LoadU8(rec);
    if (state == 0u)
        return;
    if (state == 1u) {
        PE_StoreU8(rec + 0x0Cu, 0u);
        PE_StoreU8(rec + 0x0Du, 0u);
        flags = PE_LoadU32(rec + 0x0Cu) & 0xFFF0FFFFu;
        PE_StoreU32(rec + 0x0Cu, flags);
        stream = PE_LoadU32(GA_D_8009CE90);
        PE_StoreU32(rec + 4u, stream);
        id = (int)(int16_t)PE_LoadU16(rec + 0x10u);
        cursor = pe_37870_find_marker(stream, id);
        if (cursor == 0u)
            return;
        PE_StoreU32(rec + 4u, cursor);
    }
    cursor = PE_LoadU32(rec + 4u);
    if (!pe_37870_kseg(cursor))
        return;
    pad = PE_LoadU32(GA_D_8009D1F4);
    stop = 0;
    for (guard = 0; guard < SCAN_CAP && !stop; guard++) {
        if (!pe_37870_kseg(cursor))
            break;
        byte = PE_LoadU8(cursor);
        if (byte == 0xF9u) {
            PE_StoreU8(rec, 0u);
            stop = 1;
            break;
        }
        if (byte == 0xFFu) {
            state = PE_LoadU8(rec);
            if (state == 2u && (pad & 0x100u) != 0u &&
                (PE_LoadU32(rec + 0x0Cu) & 0x02000000u) == 0u)
                PE_StoreU8(rec, 0u);
            stop = 1;
            break;
        }
        if (byte == 0xF7u) {
            cursor++;
            stop = 1;
            break;
        }
        if (byte == 0xF8u) {
            if (PE_LoadU8(rec) == 2u && (pad & 0x100u) != 0u) {
                do {
                    cursor++;
                } while (pe_37870_kseg(cursor) && PE_LoadU8(cursor) != 0xF8u);
                if (pe_37870_kseg(cursor))
                    cursor++;
                PE_StoreU32(rec + 4u, cursor);
            }
            stop = 1;
            break;
        }
        if (byte == 0xFBu) {
            if (!pe_37870_kseg(cursor + 1u))
                break;
            sub = PE_LoadU8(cursor + 1u);
            if (sub == 6u) {
                if ((pad & 0x100u) == 0u) {
                    stop = 1;
                    break;
                }
                cursor += 2u;
                continue;
            }
            if (sub == 7u) {
                if (!pe_37870_kseg(cursor + 2u))
                    break;
                nn = PE_LoadU8(cursor + 2u);
                pause = PE_LoadU8(rec + 0x0Du);
                PE_StoreU8(rec + 0x0Cu, nn);
                if (pause < nn) {
                    PE_StoreU8(rec + 0x0Du, (uint8_t)(pause + 1u));
                    stop = 1;
                } else {
                    PE_StoreU8(rec + 0x0Du, 0u);
                    cursor += 3u;
                }
                break;
            }
            if (sub == 9u) {
                uint32_t fl;
                uint8_t count;
                int8_t cur;

                if (!pe_37870_kseg(cursor + 2u))
                    break;
                fl = PE_LoadU32(rec + 0x0Cu) | 0x00200000u;
                count = (uint8_t)(PE_LoadU8(cursor + 2u) & 7u);
                fl = (fl & 0xFE3FFFFFu) | ((uint32_t)count << 22);
                PE_StoreU32(rec + 0x0Cu, fl);
                cur = (int8_t)PE_LoadU8(0x8009CEA0u);
                if ((pad & 0x20u) != 0u) {
                    cur = (int8_t)(cur + 1);
                    if (cur >= (int8_t)count)
                        cur = (int8_t)(count - 1);
                    PE_StoreU8(0x8009CEA0u, (uint8_t)cur);
                }
                if ((pad & 0x08u) != 0u) {
                    cur = (int8_t)PE_LoadU8(0x8009CEA0u);
                    cur = (int8_t)(cur - 1);
                    if (cur < 0)
                        cur = 0;
                    PE_StoreU8(0x8009CEA0u, (uint8_t)cur);
                }
                if ((pad & 0x100u) != 0u)
                    PE_StoreU8(0x8009CEA4u, PE_LoadU8(0x8009CEA0u));
                cursor += 3u;
                continue;
            }
            cursor += 2u;
            continue;
        }
        if (byte == 0xFCu || byte == 0xFDu) {
            cursor += 2u;
            continue;
        }
        cursor++;
    }
    PE_StoreU32(rec + 4u, cursor);
    if (stop && PE_LoadU8(rec) == 1u)
        PE_StoreU8(rec, 2u);
}

void func_80037870(void)
{
    unsigned int i;

    for (i = 0; i < 4u; i++)
        pe_37870_record(GA_D_800BCEA8 + i * REC_STRIDE);
}
