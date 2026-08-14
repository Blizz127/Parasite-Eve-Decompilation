/*
 * Phase 6E-B54D — func_8006AD40: material prefix through the live CD poll.
 *
 * Full retail body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive),
 *   file offset 0x5B540.
 *
 * Implemented prefix:
 *   133 words / 532 bytes, exe 0x8006AD40–0x8006AF54 (exclusive).
 *   B54D consumes the bounded material-table suffix after B54B: pack
 *   records 2 and 3 of D_80091648, look up archive key 0xABADC06C, and
 *   walk its size-prefixed LoadImage records through the already-translated
 *   func_8007506C. The first excluded instruction is:
 *
 *       jal  func_8006E7E8              # 0x8006AF54, not consumed
 *        nop
 *
 * func_8006E1C0 is TRANSLATED (Phase 6E-B51), and B52 translates its two
 * func_8007506C (Psy-Q LoadImage) wrappers through the read-only validator.
 * B53I-D completes the accepted two-LoadImage lifecycle for entry 0. B54A
 * proves that 0x8006AE50 is mid-loop rather than a dependency boundary, so
 * B54B issues the remaining entries through the same translated helper and
 * stops at the loop exit. If the initial entry count is zero, retail bypasses
 * the loop and reaches that same 0x8006AE68 boundary directly.
 *
 * Classification: 1 — translated retail prefix with an honest live CD-poll
 * boundary. No poll result, CD progression, or DMA checkpoint is invented.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_D_800930EA  0x800930EAu
#define GA_D_800930EC  0x800930ECu
#define GA_D_80091648  0x80091648u
#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_800B0DD8  0x800B0DD8u

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);

int func_8006AD40(void)
{
    pe_addr_t lba_base;
    int status;

    /* 0x8006AD6C–0x8006AD7C: guarded zero return. */
    if (!(PE_LoadU32(GA_D_800B0CD8) & 1u))
        return 0;

    lba_base = PE_LoadU32(GA_D_800B0DD8);

    /* 0x8006AD88–0x8006ADD0: channel 1 issue/poll loop. */
    for (;;) {
        do {
            uint32_t start = PE_LoadU16(GA_D_800930EA);
            uint32_t end = PE_LoadU16(GA_D_800930EA + 2u);
            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x160u),
                (int)(end - start));
        } while (status == -1);

        status = func_8006E7E8();
        if (status == 0)
            break;
        if (status != -1) {
            do {
                status = func_8006E7E8();
            } while (status != 0 && status != -1);
            if (status == 0)
                break;
        }
    }

    /* 0x8006ADD8–0x8006AE00: channel 2 issue loop. */
    do {
        uint32_t start = PE_LoadU16(GA_D_800930EC);
        uint32_t end = PE_LoadU16(GA_D_800930EC + 2u);
        status = func_8006E6A8(
            (int)(lba_base + start),
            PE_LoadU32(GA_D_800B0CD8 + 0x174u),
            (int)(end - start));
    } while (status == -1);

    /* 0x8006AE10–0x8006AE64: complete first counted texture-entry loop.
     * The header is at base + lw(base+4) + 0x28, not base + 0x28. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t metadata = base + PE_LoadU32(base + 4u);
        uint32_t header = PE_LoadU32(metadata + 0x28u);
        uint32_t count = header >> 22;
        pe_addr_t entry = base + (header & 0x003FFFFFu);
        uint32_t issued = 0u;

        if (count != 0u) {
            for (;;) {
                uint32_t continue_loop;

                func_8006E1C0(entry, base);        /* 0x8006AE48 */
                header = PE_LoadU32(metadata + 0x28u); /* 0x8006AE50 */
                issued += 1u;                      /* 0x8006AE54 */
                count = header >> 22;              /* 0x8006AE58 */
                continue_loop = issued < count;    /* sltu 0x8006AE5C */
                entry += 0x14u;                    /* delay 0x8006AE64 */
                if (!continue_loop)
                    break;
            }
        }
    }

    /* 0x8006AE68..0x8006AEF4: pack records 2 and 3 only. */
    {
        uint32_t record_offset;

        for (record_offset = 0x20u; record_offset < 0x40u;
             record_offset += 0x10u) {
            uint32_t src_a = PE_LoadU16(GA_D_80091648 + record_offset);
            uint32_t src_b = PE_LoadU16(GA_D_80091648 + record_offset + 2u);
            uint32_t src_c = PE_LoadU16(GA_D_80091648 + record_offset + 4u);
            uint32_t src_d = PE_LoadU16(GA_D_80091648 + record_offset + 6u);
            uint32_t packed1 = ((src_a & 0x3FFu) >> 6) | 0x20u;
            uint32_t packed2;

            packed1 |= (src_b & 0x100u) >> 4;
            packed1 |= (src_b & 0x200u) << 2;
            packed2 = (src_d << 6) | ((src_c >> 4) & 0x3Fu);
            PE_StoreU16(GA_D_80091648 + record_offset + 8u,
                        (uint16_t)packed1);
            PE_StoreU16(GA_D_80091648 + record_offset + 0xAu,
                        (uint16_t)packed2);
        }
    }

    /* 0x8006AEF8..0x8006AF50: the existing archive lookup and one-or-more
     * size-prefixed LoadImage records. Canonical Disc 1 has exactly one. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t record = func_8006E498(base, 0xABADC06Cu);

        while (PE_LoadU32(record) != 0u) {
            RECT rect;
            uint32_t size = PE_LoadU32(record);

            rect.x = (int16_t)PE_LoadU16(record + 4u);
            rect.y = (int16_t)PE_LoadU16(record + 6u);
            rect.w = (int16_t)PE_LoadU16(record + 8u);
            rect.h = (int16_t)PE_LoadU16(record + 0xAu);
            (void)func_8007506C(&rect, record + 0xCu);
            record += size & ~3u;
        }
    }

    /* B54D prefix cut at retail 0x8006AF54, before consuming the live
     * func_8006E7E8 poll. Keep the established provider name so strict and
     * frontier tooling retain one stable identity for this function. */
    (void)Bootstrap_ReturnInt(
        "func_8006AD40_prefix_cut", "func_8006AD40", 0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
