/*
 * PE-BTL20 — 16910 opcode 0xED live key-2900 cut.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80016910 — 314 words 0x80016910..0x80016DF8, SHA-256
 * 273744b7…8c37. D_800910A0[0xED]. Switch on *arg0.
 * Live type-1 +0x048 is key 0xB54 = 2900 → 0x80016D00:
 *   D_800B0CD8 |= 0x00400000
 *   v0=1
 * Other keys (including 35038/E00CC/6914C arms) are not this cut.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B0CD8 0x800B0CD8u

int func_80016910_key2900_cut(pe_addr_t args)
{
    uint32_t key;

    key = PE_LoadU32(PE_LoadU32(args));
    if (key == 2900u)
        PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) | 0x00400000u);
    return 1;
}
