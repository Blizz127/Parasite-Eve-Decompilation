/*
 * PE-SAVEWRITE — func_80042020 save-write entry + func_80042170 load entry
 * (hand-translated, nonmatching).
 *
 * Authority: retail Disc1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b,
 * asm/disc1/307CC.s:
 *   func_80042020  0x80042020..0x80042170 (0x150, 84w)
 *   func_80042170  0x80042170..0x80042228 (0x0B8, 46w)
 *
 * func_80042020 formats the file name via func_80071A84 (host PE_FormatterFrame,
 * routed to D_8009EE70), assembles the 0x2000-byte save block with
 * func_80040B80, and arms the record for the card write.  func_80042170 is the
 * matching load entry.
 *
 * The formatter needs a caller stack; retail's is func_80042020's own 0x30-byte
 * frame.  The port has no guest stack, so GA_FMT_SP is a dedicated host scratch
 * in the free 0x801FF040..0x801FFE00 band (PE_FormatterFrame carves
 * caller_sp-0x250..caller_sp+0x14).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_CARD_RECORD  0x800A0ED4u
#define GA_SAVE_RECORD  0x8009EE70u
#define GA_FMT_SP       0x801FF600u

void func_80042020(int card, int slot)
{
    uint32_t c = (uint32_t)card;
    pe_addr_t s1 = GA_CARD_RECORD + c * 0x418u;
    pe_addr_t s3;
    uint8_t s2 = PE_LoadU8(s1);
    uint32_t v1;
    int32_t v0;

    if (s2 != 1u)
        return;

    s3 = s1 + (uint32_t)(slot & 0xFF) * 0x44u + 0x1Cu;

    func_80042798();
    func_8005C25C();

    v0 = (int32_t)PE_LoadU8(s3);
    v1 = (uint32_t)PE_LoadU8(s1 + 0xAu);
    if (v0 == (int32_t)s2)
        v0 = (int32_t)(v1 + 1u);
    else
        v0 = (int32_t)v1;
    PE_StoreU8(s1 + 3u, (uint8_t)slot);
    v1 = (uint32_t)(slot & 0xFF);
    PE_StoreU8(s1 + 0xAu, (uint8_t)v0);

    {
        uint32_t saved[9];
        uint32_t variant = (uint32_t)PE_LoadU8(s1 + v1 * 0x44u + 0x45u) + 0x30u;
        for (int i = 0; i < 9; i++)
            saved[i] = 0u;
        saved[0] = (uint32_t)slot;      /* s0 */
        saved[1] = s1;                  /* s1 */
        saved[2] = s2;                  /* s2 */
        saved[3] = s3;                  /* s3 */
        saved[8] = 0x800420E8u;         /* ra */
        PE_StoreU32(GA_FMT_SP + 16u, v1 + 0x41u);
        (void)PE_FormatterFrame(GA_SAVE_RECORD, PE_LoadU32(0x80092224u),
                                (GA_CARD_RECORD < s1) ? 1u : 0u, variant,
                                GA_FMT_SP, saved);
    }

    func_80040B80(GA_SAVE_RECORD);

    PE_StoreU8(s1 + 5u, (uint8_t)slot);
    PE_StoreU8(s1 + 6u, 0u);
    PE_StoreU8(s3 + 1u, 0u);
    PE_StoreU16(s1 + 0x14u, 0x2000u);
    PE_StoreU16(s1 + 0x16u, 0x0Au);
    PE_StoreU8(s1 + 1u, 1u);
    PE_StoreU8(s1 + 7u, 1u);
    PE_StoreU32(0x800A1854u, s1);
    PE_StoreU32(0x800A1858u, 0x2000u);

    v0 = (int32_t)PE_LoadU8(s3);
    PE_StoreU8(s1 + 0xBu, (uint8_t)(v0 == 1 ? 0xBu : 0x4u));
    PE_StoreU8(s3, 1u);
}

int32_t func_80042170(int card, int32_t slot)
{
    pe_addr_t s0 = GA_CARD_RECORD + (uint32_t)card * 0x418u;
    int32_t s1 = slot;

    if (PE_LoadU8(s0) != 1u)
        return 0;
    func_80042798();
    PE_StoreU16(s0 + 0x14u, 0x2000u);
    PE_StoreU16(s0 + 0x16u, 0x0Au);
    PE_StoreU8(s0 + 7u, 2u);
    PE_StoreU8(s0 + 1u, 1u);
    PE_StoreU8(s0 + 0xBu, 5u);
    PE_StoreU32(s0 + 0x18u, 0x8009EED0u);
    PE_StoreU8(s0 + 3u, (uint8_t)s1);
    PE_StoreU32(0x800A1854u, s0);
    PE_StoreU32(0x800A1858u, 0x2000u);
    func_80071A24(0x8009EED0u, 0x2000u);
    return 0;
}
