/*
 * PE-BTL21 — live type-1 0xE1 / 0x84 / 0x88 (translated retail,
 * not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001A374 — 7 words 0x8001A374..0x8001A390. sb *arg0 →
 * D_800BCFFC. v0=1. Live imm 0x54.
 *
 * func_80018E84 — 12 words 0x80018E84..0x80018EB4. sh *arg0 →
 * D_800CD020, sh *arg1 → D_800CD022. v0=1. Live 0x800, 0x800.
 *
 * func_80018F54 — 8 words 0x80018F54..0x80018F74.
 * D_800BCFEE &= ~0x40. v0=1.
 *
 * After these, type-1 0x14/0x02 are already ported and yield.
 * Next visit is 0x08/1735C (type-0 spawn) — not this cut.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_8001A374(pe_addr_t args)
{
    PE_StoreU8(0x800BCFFCu, (uint8_t)PE_LoadU32(PE_LoadU32(args)));
    return 1;
}

int func_80018E84(pe_addr_t args)
{
    PE_StoreU16(0x800CD020u, (uint16_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU16(0x800CD022u, (uint16_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

int func_80018F54(pe_addr_t args)
{
    (void)args;
    PE_StoreU8(0x800BCFEEu, (uint8_t)(PE_LoadU8(0x800BCFEEu) & 0xBFu));
    return 1;
}
