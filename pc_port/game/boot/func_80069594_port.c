/*
 * PE-BTL53 — 35558 epilogue event pump 69594 / 6F8EC / D4704
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 69594: 50 words 0x80069594..0x8006965C, SHA-256 b182bcd8…ac47.
 * Sole TEXT jal 35558 @ 0x80035B2C. If D1A0&0x80, walks
 * slots 0..10 through 6F8EC. 661A4/661CC GTE bracket and
 * the 6F9F0 follow-up are not this cut.
 *
 * 6F8EC: 65 words 0x8006F8EC..0x8006F9F0, SHA-256 659a9a3e…6563.
 * Jalrs table[remap(slot+1)]+0xC. Live 0x75 → D4704.
 *
 * D4704: 83 words 0x800D4704..0x800D4850, SHA-256 4090a71a…62ef.
 * Writes F32D0/E2368. Eight records at slot+0x2C are 0xFFFF
 * after D4620, so the GTE/jalr body is skipped. 6DC18 when
 * slot+0x19!=0 is not this cut (live +0x19=0).
 *
 * This pump does not write 2FAF8 clip rec=4. That producer
 * is 1F4D4 @ 1F5A4 (slot+0x18). Do not invent rec=4.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800942E0 0x800942E0u
#define GA_D_800942E4 0x800942E4u
#define GA_D_800942E8 0x800942E8u
#define GA_D_800F32D0 0x800F32D0u
#define GA_D_800E2368 0x800E2368u
#define GA_FN_D4704   0x800D4704u

extern unsigned int D_8009D1A0;

int func_800D4704(pe_addr_t slot)
{
    unsigned int i;
    pe_addr_t rec;

    PE_StoreU32(GA_D_800F32D0, slot);
    PE_StoreU32(GA_D_800E2368, slot + 0x0Cu);
    rec = slot + 0x2Cu;
    for (i = 0; i < 8u; i++) {
        if (PE_LoadU16(rec) != 0xFFFFu)
            return 0;
        rec += 0x0Cu;
    }
    return 0;
}

int func_8006F8EC(unsigned int index)
{
    pe_addr_t pool;
    pe_addr_t slot;
    pe_addr_t table;
    pe_addr_t entry;
    pe_addr_t fn;
    unsigned int used;
    unsigned int code;
    unsigned int stride;

    if (index >= 0x16u)
        return -16;
    if (index < 0xBu) {
        pool = PE_LoadU32(GA_D_800942E4);
        stride = 0xA0Cu;
        slot = pool + index * stride;
    } else {
        pool = PE_LoadU32(GA_D_800942E8);
        stride = 0x10Cu;
        slot = pool + (index - 0xBu) * stride;
    }

    used = PE_LoadU8(slot);
    if ((used - 1u) >= 2u)
        return 0;
    code = PE_LoadU8(slot + 1u);
    if (code >= 0xC0u)
        return -17;
    if (code >= 0x55u)
        code = 0x55u;
    table = PE_LoadU32(GA_D_800942E0);
    entry = PE_LoadU32(table + code * 4u);
    if (entry == 0u)
        return -18;
    fn = PE_LoadU32(entry + 0x0Cu);
    if (fn == 0u)
        return -1;
    if (fn == GA_FN_D4704)
        return func_800D4704(slot);
    return 0;
}

int func_80069594(void)
{
    unsigned int i;

    if ((D_8009D1A0 & 0x80u) == 0u)
        return 0;
    for (i = 0; i < 11u; i++)
        (void)func_8006F8EC(i);
    return 0;
}
