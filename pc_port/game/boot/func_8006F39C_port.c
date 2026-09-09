/*
 * PE-BTL48 — func_8006F39C event start plus 0x55 helpers
 * (translated retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 6F39C: 206 words 0x8006F39C..0x8006F6D4, SHA-256 ee236f21…8337.
 * CE49C: 23 words 0x800CE49C..0x800CE4F8, SHA-256 65cc4cd5…63bc.
 * D4620: 30 words 0x800D4620..0x800D4698, SHA-256 52e49e7d…10a5.
 *
 * Opcode 0x6A / 18774 jals 6F39C(*arg0, D2F0). Live imm 0x75
 * remaps to table code 0x55 (D_800942E0[0x55] = 0x800E13D4,
 * +4 = 0x800D4620). Finds a free 0xA0C slot in *D_800942E4
 * (6914C / 6A8D4 B0E60). CE49C(slot, 0x20) is the 0x55 extra;
 * EXE D_800E1044[0x20] is 0 so that leaf returns -1 without
 * writing +0x8C. D4620 then inits the slot. Return is the
 * slot index.
 *
 * Codes 0x6C..0x72 CD/XA prelude is not this cut (live 0x75
 * skips it). Unknown jalr targets are not invented.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800942E0 0x800942E0u
#define GA_D_800942E4 0x800942E4u
#define GA_D_800942E8 0x800942E8u
#define GA_D_800E1044 0x800E1044u
#define GA_FN_D4620   0x800D4620u

void func_800D4620(pe_addr_t slot)
{
    pe_addr_t a1;
    pe_addr_t a0;
    pe_addr_t a2;
    int i;

    a2 = slot + 0x0Cu;
    PE_StoreU8(slot + 0x02u, 1u);
    PE_StoreU8(slot + 0x03u, 0u);
    PE_StoreU16(slot + 0x1Au, 0u);
    PE_StoreU16(slot + 0x1Cu, 0u);
    PE_StoreU32(slot + 0x10u, slot + 0x90u);
    PE_StoreU8(slot + 0x18u, 0u);
    PE_StoreU8(slot + 0x19u, 0u);
    a1 = slot + 0x18u;
    for (i = 6; i >= 0; i--) {
        PE_StoreU16(a1 + 0x12u, 0u);
        a1 -= 2u;
    }
    a1 = a2 + 0x20u;
    a0 = a2 + 0x24u;
    for (i = 0; i < 8; i++) {
        PE_StoreU16(a1, 0xFFFFu);
        PE_StoreU16(a0 - 2u, 0u);
        PE_StoreU32(a0, 0u);
        a0 += 0x0Cu;
        a1 += 0x0Cu;
    }
}

int func_800CE49C(pe_addr_t slot, unsigned int extra)
{
    pe_addr_t rec;

    rec = PE_LoadU32(GA_D_800E1044 + extra * 4u);
    if (rec == 0u)
        return -1;
    PE_StoreU32(slot + 0x8Cu, rec);
    PE_StoreU32(slot + 0x0Cu, PE_LoadU32(rec + 0x34u));
    return 0;
}

int func_8006F39C(unsigned int code, pe_addr_t userdata)
{
    unsigned int orig;
    unsigned int extra;
    pe_addr_t table;
    pe_addr_t entry;
    pe_addr_t fn;
    pe_addr_t pool;
    pe_addr_t slot;
    unsigned int stride;
    int index;
    int i;
    uint8_t used;

    orig = code;
    extra = 0u;
    if (code >= 0xC0u)
        return -7;

    (void)func_8006914C(0);

    if (code >= 0x55u) {
        extra = code - 0x55u;
        code = 0x55u;
    }

    table = PE_LoadU32(GA_D_800942E0);
    entry = PE_LoadU32(table + code * 4u);
    if (entry == 0u)
        return -8;
    fn = PE_LoadU32(entry + 4u);
    if (fn == 0u)
        return -1;

    if (code >= 0x46u && code < 0x55u) {
        pool = PE_LoadU32(GA_D_800942E8);
        stride = 0x10Cu;
    } else {
        pool = PE_LoadU32(GA_D_800942E4);
        stride = 0xA0Cu;
    }

    index = -1;
    slot = pool;
    for (i = 0; i < 11; i++) {
        used = PE_LoadU8(slot);
        if (used == 0u) {
            index = i;
            break;
        }
        slot += stride;
    }
    if (index < 0)
        return -3;

    slot = pool + (pe_addr_t)index * stride;
    PE_StoreU8(slot, 1u);
    PE_StoreU8(slot + 1u, (uint8_t)orig);
    PE_StoreU8(slot + 2u, 0u);
    PE_StoreU8(slot + 3u, 0u);
    PE_StoreU32(slot + 4u, 0u);
    PE_StoreU32(slot + 8u, userdata);
    if (code == 0x55u)
        (void)func_800CE49C(slot, extra);
    if (fn == 0x8018EFFCu && PE_MirrorOverlay())
        (void)PE_MirrorInit(slot);
    else if (fn == GA_FN_D4620)
        func_800D4620(slot);
    else if (fn == 0x800C9A70u)
        (void)func_800C9A70(slot);
    else if (fn == 0x800CD728u)
        (void)func_800CD728(slot);
    else {
        Bootstrap_ReturnVoid("func_8006F39C_constructor", "func_8006F39C");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
    return index;
}
