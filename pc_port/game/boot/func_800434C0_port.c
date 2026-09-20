/*
 * PE-CARD-MENU — slot-row draw callback and its text helper.
 *
 * Translated from the retail bytes, not matching leaves:
 *   func_800434C0  asm/disc1/33CC0.s  0x800434C0..0x800437B4 (189 words)
 *   func_8005DD8C  asm/disc1/4E58C.s  0x8005DD8C..0x8005DE08 (31 words)
 *
 * func_800434C0 is the `draw` callback installed by func_8004FEEC for the
 * "Select Slot" list.  It is invoked per cell with the item index, looks up the
 * slot entry (func_800424B4), and lays out the row by entry type:
 *   type 1  the occupied save slot (name, time, level, screenshot)
 *   type 2  an empty slot
 *   type 3  the "new file" row
 * Every cursor move is a func_8005E8A4(x,y); every glyph/sprite is a
 * func_8005EB64 / func_8005F27C / func_8005FB74 call, exactly as retail.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8005DD8C(index) -> text pointer, or NULL.  String table at
 * D_800A8028: byte index map at D_800A8028[D_800A804C + index], and the
 * per-string offsets in the table at D_800A8028 + [D_800A802C]. */
pe_addr_t func_8005DD8C(int32_t index)
{
    uint32_t table = PE_LoadU32(0x800A804Cu);
    uint8_t idx = PE_LoadU8(0x800A8028u + (uint32_t)index + table);
    uint32_t base;
    pe_addr_t tbl;
    uint32_t n;

    if (idx == 0u) return 0u;
    base = PE_LoadU32(0x800A802Cu) + 0x800A8028u;
    tbl = (pe_addr_t)(base + PE_LoadU32(base + 0x10u));
    n = (uint32_t)idx + 0x7Fu;
    if (n >= PE_LoadU16(tbl)) return 0u;
    return (pe_addr_t)(tbl + (int16_t)PE_LoadU16(tbl + n * 2u + 2u));
}

void func_800434C0(pe_addr_t item)
{
    pe_addr_t entry = func_800424B4(
        PE_LoadU32(PE_LoadU32(0x8009CEF4u) + 0x24u) - 0x25u, (int32_t)item);
    uint32_t type;

    if (entry == 0u) return;
    type = PE_LoadU8(entry);
    if (type == 2u) goto empty_slot;
    if (type < 3u) {
        if (type == 1u) goto occupied_slot;
        return;                                 /* type 0 */
    }
    if (type == 3u) {
        func_8005E8A4(0, 0x12);
        func_80064C54(0x76u);
        func_8005E8A4(0x5C, -0x10);
        return;
    }
    return;                                     /* type > 3 */

occupied_slot:
    if (item == func_80063428(PE_LoadU32(0x8009CEF4u))) {
        func_800622B0((int)PE_LoadU32(entry + 0x20u));
    } else {
        func_8005E8A4(-2, -2);
        (void)func_800614AC((int)PE_LoadU32(entry + 0x20u));
        {
            pe_addr_t node = PE_LoadU32(0x8009CEF4u);
            func_80061A3C(PE_LoadU32(node + 0x3Cu),
                          PE_LoadU32(node + 0x40u), 1u);
        }
        (void)func_800614AC((int)PE_LoadU32(0x800C0E44u));
        func_8005E8A4(2, 2);
    }
    func_8005E8A4(2, 2);
    func_8005F27C(func_8005BCB0() ? entry + 0x14u : entry + 0x4u);
    func_8005E8A4(0x58, 0);
    if (PE_LoadU8(entry + 0x2Au) != 0u) {
        func_8005EB64(0x95u);
        func_8005E8A4(0x1A, 2);
        func_8005EB64(0x96u);
        func_8005E8A4(0x22, 1);
        func_8005FA3C((int32_t)PE_LoadU8(entry + 0x2Au) + 1);
        func_8005E8A4(4, -3);
    } else {
        func_8005E8A4(0x4A, 0);
    }
    func_8005F5B8(0x5Au);
    func_8005E8A4(0x19, 0);
    func_800605F8((int32_t)item + 1);
    func_8005E8A4(-0xC4, 0xF);
    func_8005EB64(0x53u);
    func_8005E8A4(0x1E, 1);
    func_8005FB74((int32_t)PE_LoadU8(entry + 0x28u) + 1);
    func_8005E8A4(8, -1);
    func_8005EB64(0x54u);
    func_8005E8A4(0x10, 1);
    func_8005FDF0((int32_t)(int16_t)PE_LoadU16(entry + 0x24u));
    func_8005EB64(0x4Cu);
    func_8005E8A4(5, 0);
    func_8005FDF0((int32_t)(int16_t)PE_LoadU16(entry + 0x26u));
    func_8005E8A4(8, -1);
    func_8005EB64(0x55u);
    func_8005E8A4(0x1A, 1);
    func_8006006C((int32_t)PE_LoadU32(entry + 0xCu), 0u);
    func_8005E8A4(-0xC0, 0xC);
    if (PE_LoadU8(entry + 0x29u) != 0u) {
        func_8005F5B8(0x60u);
        return;
    }
    func_8005F5B8(0xBu);
    func_8005E8A4(0x1E, 0);
    func_800605F8((int32_t)(int16_t)PE_LoadU16(entry + 0x2Cu));
    func_8005E8A4(0x14, 0);
    func_8005F27C(func_8005DD8C((int32_t)(int16_t)PE_LoadU16(entry + 0x2Eu)));
    return;

empty_slot:
    func_8005E8A4(0, 0x12);
    func_80064C54(0x41u);
    func_8005E8A4(0x5C, -0x10);
}
