/*
 * PE-CARD-MENU — slot-list window constructor and unable path.
 *
 * Translated from the retail bytes, not matching leaves:
 *   func_8004D4C4  asm/disc1/3DCC4.s  0x8004D4C4..0x8004D5CC (66 words)
 *   func_8004D298  asm/disc1/3DA98.s  0x8004D298..0x8004D2DC (17 words)
 *   func_8004D690  asm/disc1/3DCC4.s  0x8004D690..0x8004D6D4 (17 words)
 *
 * These are the two func_80041108 state-2 continuations that build the
 * "Select Slot" save window (continue) or the unable notice.  They were
 * explicit boundaries because the slot-list UI chain was unported.
 *
 * gp resolves to 0x8009CD70 (proved by func_80062A34's `lw v1,996(gp)` =
 * D_8009D154, the menu node-list head), so:
 *   0x1D4(gp) = D_8009CF44   (func_8004D4C4 stores the record index there)
 *   0x1E0(gp) = D_8009CF50   (func_8004D298 reads the unable-string base)
 * Both addresses are already used by this port (func_8004D5CC /
 * func_8004DF74), which independently confirms the gp base.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

extern void func_8005DE88(void);
/* Matching decomp leaves without a shared prototype. */
extern void func_8003FFAC(int value);
extern int  func_80042964(int index);

/* func_8004D4C4(idx, items) — continue path slot-list constructor.
 * Retail order: look up the (2,0x24) parent, allocate the list node, allocate
 * its (idx+0x25) child row, install the row draw callbacks, lay out the items,
 * restore the saved cursor or free the empty row, then create the 0x3F prompt
 * node with the func_8004D690 callback and publish the selected index to
 * D_8009CF44.  Returns func_80063428's result (the caller stores its low byte
 * at record+5). */
int32_t func_8004D4C4(uint32_t idx, uint32_t items)
{
    pe_addr_t parent = func_80062A34(2u, 0x24u);
    pe_addr_t node = func_80062D2C(idx + 0x25u, parent, 0u, 0u);
    pe_addr_t child = func_8006322C(idx + 0x25u, node, node);
    int32_t result;

    PE_StoreU32(node + 0x2Cu, 0x8004D6D4u);
    PE_StoreU32(child + 0x30u, 0x8004FEECu);
    PE_StoreU32(child + 0x8Cu, 0x8004FE58u);

    func_800647D0(child, (int32_t)items);
    result = func_80063428(child);
    {
        int32_t saved = (int32_t)func_800631DC();
        if (saved == 0) {
            PE_StoreU32(child + 0x44u, 0u);
            func_80062CB8(child);
        } else {
            PE_StoreU32(child + 0x44u, (uint32_t)saved);
        }
    }
    {
        pe_addr_t prompt = func_80062D2C(0x3Fu, child, 0u, 0u);
        PE_StoreU32(prompt + 0x30u, 0x8004D690u);
    }
    func_8005DE88();
    PE_StoreU32(0x8009CF44u, idx);
    return result;
}

/* func_8004D298(index) — unable path.  If no (1,0x28) notice node exists,
 * build the unable window with message id index+0x47 and
 * ([D_8009CF50]+0x42) as its second string. */
void func_8004D298(uint32_t index)
{
    if (func_80062A34(1u, 0x28u) == 0u)
        func_8004CE28(index + 0x47u, PE_LoadU32(0x8009CF50u) + 0x42u);
}

/* func_8004D690(prompt) — prompt-row draw callback.  Retail reads the prompt
 * node's owner (+4) and its +0x24 field to select the help/notice string. */
void func_8004D690(pe_addr_t node)
{
    pe_addr_t owner;

    func_8005E8A4(0, 2);
    owner = PE_LoadU32(node + 4u);
    func_80062A7C(PE_LoadU32(owner + 0x24u) - 0x11u);
}

/* func_800424B4(card, item) — slot entry lookup.  Retail 0x800424B4..0x80042538
 * (33 words): card<2 and 0<=item<record[card].+2 (D_800A0ED4 + card*0x418 +
 * 2), and the entry's +0x1D flag nonzero, else NULL.  Returns the entry's
 * +0x1C type-byte address: 0x800A0EF0 + card*0x418 + item*0x44. */
pe_addr_t func_800424B4(uint32_t card, int32_t item)
{
    uint32_t stride;
    uint32_t off;

    if (card >= 2u) return 0u;
    if (item < 0) return 0u;
    stride = card * 0x418u;
    if ((uint32_t)item >= PE_LoadU8(0x800A0ED6u + stride)) return 0u;
    off = (uint32_t)item * 0x44u;
    if (PE_LoadU8(0x800A0EF1u + stride + off) == 0u) return 0u;
    return 0x800A0EF0u + stride + off;
}

/* func_8004FEEC(list) — the slot-list node's +0x30 draw callback.  Retail
 * 0x8004FEEC..0x8004FF30 (17 words): publish the node, draw the list with the
 * func_800434C0 row callback, then flush the menu frame. */
void func_8004FEEC(pe_addr_t list)
{
    PE_StoreU32(0x8009CEF4u, list);
    func_800638D8(list, 0x800434C0u);
    (void)func_800614AC((int)PE_LoadU32(0x800C0E44u));
    func_800622B0(0);
}

/* func_8004FE58(item) — the slot row's +0x8C state predicate.  Retail
 * 0x8004FE58..0x8004FEEC (37 words): with D_8009CF50 set, "entry type != 3";
 * otherwise "entry type == 1 and entry[0x29] == func_8003FFBC()". */
int func_8004FE58(pe_addr_t item)
{
    pe_addr_t entry = func_800424B4(
        PE_LoadU32(PE_LoadU32(0x8009CEF4u) + 0x24u) - 0x25u, (int32_t)item);

    if (PE_LoadU32(0x8009CF50u) != 0u)
        return (PE_LoadU8(entry) ^ 3u) != 0u;
    if (PE_LoadU8(entry) != 1u) return 0;
    return (uint32_t)((PE_LoadU8(entry + 0x29u) ^
                       (uint8_t)func_8003FFBC()) < 1u);
}

/* func_8004D978(value) — build the three-line prompt/notice window.  Retail
 * 0x8004D978..0x8004D9D8 (24 words): allocate a (0x27) node under the
 * func_80062CC4() parent, install the func_8004DA04 draw callback at +0x30 and
 * the func_8004DA9C input callback at +0x2C, realise it, then publish value to
 * D_8009D000 (gp+0x290). */
void func_8004D978(uint32_t value)
{
    pe_addr_t node = func_80062D2C(0x27u, func_80062CC4(), 0u, 1u);

    PE_StoreU32(node + 0x30u, 0x8004DA04u);
    PE_StoreU32(node + 0x2Cu, 0x8004DA9Cu);
    func_80062CB8(node);
    PE_StoreU32(0x8009D000u, value);
}

/* func_8004D6D4(window, event) — slot-list window input/handler.  Retail
 * 0x8004D6D4..0x8004D978 (169 words); this is the +0x2C callback the slot-list
 * constructor func_8004D4C4 installs.  `event` is the menu event mask:
 *   0x40     close/cancel the window (func_80062F3C(0x3F) teardown, return 1)
 *   0x10000  confirm/select an entry
 * gp = 0x8009CD70, so 0x1D4/0x1D8/0x1DC/0x1E0(gp) are D_8009CF44/48/4C/50:
 * the selected record index, the func_8006346C result, the entry type byte,
 * and the string base whose zero selects the unable/notice paths.
 *
 * The confirm path looks up the slot entry via func_800424B4; type 2 (occupied)
 * either opens the 0x45 load/overwrite prompt (fewer than 0xF files) or runs
 * the func_8004CE28 notice; a zero string base runs the 0x46 unable notice.
 * The generic continue path builds the (0x29) sub-list with its layout and
 * publishes the func_80050544 scroll callback.  Every terminal arm returns 1
 * through func_800525EC (handled) or func_800526C4 (unable). */
int32_t func_8004D6D4(pe_addr_t window, uint32_t event)
{
    pe_addr_t list = func_80062A20(window, 0u);
    pe_addr_t entry;
    int32_t selected;

    if ((event & 0x40u) != 0u) {
        func_80062F3C(0x3Fu);
        func_80062F1C(window);
        func_80042A10();
        func_80052634();
        return 1;
    }
    if ((event & 0x10000u) == 0u)
        return 1;

    selected = func_8006346C(list);
    PE_StoreU32(0x8009CF48u, (uint32_t)selected);
    if (selected < 0)
        goto unable;

    entry = func_800424B4(PE_LoadU32(0x8009CF44u), selected);
    if (entry == 0u)
        goto unable;

    {
        uint32_t string_base = PE_LoadU32(0x8009CF50u);

        if (string_base == 0u && PE_LoadU8(entry) == 2u)
            goto unable;
        PE_StoreU32(0x8009CF4Cu, PE_LoadU8(entry));
        if (string_base == 0u)
            goto notice_46;
    }

    func_8003FFAC((int)PE_LoadU32(0x8009CFF8u));

    if (PE_LoadU32(0x8009CF4Cu) == 2u) {
        if (func_80042964((int)PE_LoadU32(0x8009CF44u)) < 0xF) {
            func_80062F3C(0x1Fu);
            func_8004D978(0x45u);
            func_80042B50(0x800504F4u);
        } else if (func_80062A34(1u, 0x28u) == 0u) {
            func_8004CE28(PE_LoadU32(0x8009CF44u) + 0x47u,
                          PE_LoadU32(0x8009CF50u) + 0x42u);
        }
        goto done;
    }

    /* Generic continue arm: build the (0x29) sub-list and its layout. */
    {
        pe_addr_t row = func_80062D2C(0x29u, list, 0u, 1u);
        pe_addr_t child = func_8006322C(0x29u, row, row);
        uint32_t text;
        int32_t width;

        PE_StoreU32(row + 0x30u, 0x80044E14u);
        PE_StoreU32(row + 0x2Cu, 0x80044E98u);
        PE_StoreU32(child + 0x30u, 0x8004F950u);
        PE_StoreU32(0x8009CF14u, 0x6Cu);
        func_80062CB8(child);
        PE_StoreU32(child + 0x44u, 1u);      /* retail: sw in the jal delay slot */
        func_80052E30(PE_LoadU32(0x8009CF10u));

        PE_StoreU8(0x800A1980u, 0xFFu);
        text = func_8005DC4C(0x44u);
        func_80052C08(0x800A1980u, text);
        PE_StoreU32(0x8009CFA0u, 0u);

        /* Width is measured once; below 0x78 it is clamped up, otherwise the
         * measurement is repeated and its result used (retail calls
         * func_8005F1A0 a second time). */
        width = (int32_t)func_8005F1A0(0x800A1980u);
        if (width >= 0x78)
            width = (int32_t)func_8005F1A0(0x800A1980u);
        else
            width = 0x78;
        PE_StoreU32(row + 0x34u, (uint32_t)(width + 0x14));
        PE_StoreU32(row + 0x38u, 0x32u);
        PE_StoreU32(row + 0x18u, (uint32_t)((0x12C - width) >> 1));
        PE_StoreU32(child + 0x18u,
                    (uint32_t)(((int32_t)PE_LoadU32(row + 0x34u) - 0x80) >> 1));
        PE_StoreU32(0x8009CFA8u, 0x80050544u);
        PE_StoreU32(child + 0x1Cu, PE_LoadU32(row + 0x38u) - 0x14u);
    }
    goto done;

notice_46:
    func_80062F3C(0x1Fu);
    func_8004D978(0x46u);
    func_80042B50(0x8005051Cu);

done:
    func_800525EC();
    return 1;

unable:
    func_800526C4();
    return 1;
}
