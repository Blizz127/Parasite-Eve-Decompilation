/*
 * Phase 6E-B54F — func_8006AD40: D_800930EE issue, 718D0, record 0/1.
 *
 * Full retail body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive),
 *   file offset 0x5B540.
 *
 * Implemented prefix:
 *   200 words / 800 bytes, exe 0x8006AD40–0x8006B060 (exclusive).
 *   B54G consumes 0x8006B04C..0x8006B060: the second live
 *   func_8006E7E8 wait/reissue, B54E-shaped. Exclusive end is the
 *   poll==0 fallthrough. The first excluded instruction is:
 *
 *       addu  s0, zero, zero            # 0x8006B060
 *
 * func_8006E1C0 is TRANSLATED (Phase 6E-B51), and B52 translates its two
 * func_8007506C (Psy-Q LoadImage) wrappers through the read-only validator.
 * B53I-D completes the accepted two-LoadImage lifecycle for entry 0. B54A
 * proves that 0x8006AE50 is mid-loop rather than a dependency boundary, so
 * B54B issues the remaining entries through the same translated helper and
 * stops at the loop exit. If the initial entry count is zero, retail bypasses
 * the loop and reaches that same 0x8006AE68 boundary directly.
 *
 * Classification: 1 — translated retail prefix. The second poll is
 * consumed live; poll/s2 are not assigned. Host D_8009B6B4 collapse
 * at the D_800930EE issue is B54E-HOST-POLL-COLLAPSE, not retail
 * timing. The suffix now issues D_800930F0, walks the prior image, and
 * enters func_80030894. B54K-A/B1/B2/B3 translate that 788-word builder
 * through exclusive address 0x80030F6C (438 words), with every reached GPU
 * helper native. The first unresolved inner boundary is the next packet
 * group, named func_80030894_L6_cut.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_D_800930EA  0x800930EAu
#define GA_D_800930EC  0x800930ECu
#define GA_D_800930EE  0x800930EEu
#define GA_D_800930F0  0x800930F0u
#define GA_D_80091648  0x80091648u
#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_800B0DD8  0x800B0DD8u

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern pe_addr_t func_800718D0(pe_addr_t tim);
extern void func_80030894(void);

/* Shared GetTPage/GetClut pack used by B54D records 2/3 (a1=0x20,0x30)
 * and by the B54C font sites at 0x8006AFF8 / 0x8006B02C (a1=0,0x10). */
static void pack_d80091648(uint32_t record_offset)
{
    uint32_t src_a = PE_LoadU16(GA_D_80091648 + record_offset);
    uint32_t src_b = PE_LoadU16(GA_D_80091648 + record_offset + 2u);
    uint32_t src_c = PE_LoadU16(GA_D_80091648 + record_offset + 4u);
    uint32_t src_d = PE_LoadU16(GA_D_80091648 + record_offset + 6u);
    uint32_t packed1 = ((src_a & 0x3FFu) >> 6) | 0x20u;
    uint32_t packed2;

    packed1 |= (src_b & 0x100u) >> 4;
    packed1 |= (src_b & 0x200u) << 2;
    packed2 = (src_d << 6) | ((src_c >> 4) & 0x3Fu);
    PE_StoreU16(GA_D_80091648 + record_offset + 8u, (uint16_t)packed1);
    PE_StoreU16(GA_D_80091648 + record_offset + 0xAu, (uint16_t)packed2);
}

void PE_func_8006AD40_PackFontRecords(void)
{
    uint32_t record_offset;

    /* 0x8006AFB0..0x8006B038: a1 = 0, 0x10; a1 < 0x20. */
    for (record_offset = 0u; record_offset < 0x20u; record_offset += 0x10u)
        pack_d80091648(record_offset);
}

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
             record_offset += 0x10u)
            pack_d80091648(record_offset);
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

    /* 0x8006AF44..0x8006AF68: wait/reissue around live func_8006E7E8.
     * Canonical entry locals are s0=1 (AF44) and s2=1 (AE04). AE08 with
     * s0!=0 is the wait head, not the texture loop. Do not assign s2. */
    status = 1;
    for (;;) {
        if (status == -1) {
            /* 0x8006ADD8: reissue the same channel-2 range. */
            do {
                uint32_t start = PE_LoadU16(GA_D_800930EC);
                uint32_t end = PE_LoadU16(GA_D_800930EC + 2u);
                status = func_8006E6A8(
                    (int)(lba_base + start),
                    PE_LoadU32(GA_D_800B0CD8 + 0x174u),
                    (int)(end - start));
            } while (status == -1);
            status = 1; /* 0x8006AE04 */
        }
        status = func_8006E7E8(); /* 0x8006AF54 — live result */
        if (status == 0)
            break;
        /* 0x8006AE08: s0==1 branches to 0x8006AF4C. */
    }

    /* 0x8006AF68..0x8006B04C: issue D_800930EE / dest+0x180, walk the
     * already-complete TIM at dest+0x174, pack records 0 and 1. Canonical
     * s0 is 0 here so 718D0 runs. s2 is set to 1 after a successful issue,
     * so the B044 s2==-1 reissue is not taken. Do not poll at B04C. */
    {
        int s0 = 0; /* 0x8006AF68 */

        do {
            uint32_t start = PE_LoadU16(GA_D_800930EE);
            uint32_t end = PE_LoadU16(GA_D_800930EE + 2u);

            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x180u),
                (int)(end - start));
        } while (status == -1);

        status = 1; /* 0x8006AF98 */
        if (s0 == 0) {
            (void)func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x174u));
            PE_func_8006AD40_PackFontRecords();
            s0 = 1; /* 0x8006B040 */
        }
        /* 0x8006B044 beq s2, -1, AF6C — not taken (status == 1). */
        (void)s0;
    }

    /* 0x8006B04C..0x8006B060: wait/reissue around live func_8006E7E8
     * for the D_800930EE issue. Canonical locals are s0=1 (B040) and
     * s2=1 (AF98). AF9C with s0!=0 is the wait head (B044), not 718D0.
     * Do not assign s2. Do not issue D_800930F0. */
    for (;;) {
        if (status == -1) {
            /* 0x8006AF6C: reissue the same D_800930EE range. */
            do {
                uint32_t start = PE_LoadU16(GA_D_800930EE);
                uint32_t end = PE_LoadU16(GA_D_800930EE + 2u);
                status = func_8006E6A8(
                    (int)(lba_base + start),
                    PE_LoadU32(GA_D_800B0CD8 + 0x180u),
                    (int)(end - start));
            } while (status == -1);
            status = 1; /* 0x8006AF98 */
        }
        status = func_8006E7E8(); /* 0x8006B04C — live result */
        if (status == 0)
            break;
        /* 0x8006AF9C: s0==1 branches to 0x8006B044. */
    }

    /* B54K-A/B1/B2/B3: issue D_800930F0 into dest+0x14C. Do not poll — the
     * busy bits stay armed across the current 30894 L6 cut. 718D0 walks
     * the prior 930EE dest at +0x180 (zero TIM on the prefix path). */
    {
        uint32_t start = PE_LoadU16(GA_D_800930F0);
        uint32_t end = PE_LoadU16(GA_D_800930F0 + 2u);

        do {
            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x14Cu),
                (int)(end - start));
        } while (status == -1);
        (void)func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x180u));
    }

    func_80030894();
    (void)Bootstrap_ReturnInt(
        "func_8006AD40_post30894_cut", "func_8006AD40", 0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
