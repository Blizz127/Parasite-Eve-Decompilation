/*
 * PE-CH3 — field message subsystem, ported for field-VM opcode 0xE9.
 * Authority: SHA-1-exact retail EXE
 * (build/extracted/disc1/SLUS_006.62, sha1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * Translated (not matching src/) from:
 *   41D10.s  func_800515C0 (0x38)          — message record +0xC setter
 *   3420.s   func_800629B0                 — "message queued" flag leaf
 *   48530.s  func_80057E14 (0xB8)          — window-id remap table
 *   42D94.s  func_8005270C (0x58)          — window fade target
 *   3BD84.s  func_8004BE4C (0xBC)          — "item get" window node pair
 *   3BD84.s  func_8004BCE8 (0x164)         — close/commit active window
 *   4CC98.s  func_8005D2B4 (0x440)         — 21-arm message dispatcher
 *   3420.s   func_80015C7C (0x130)         — field-VM opcode 0xE9
 *   field leaves func_80051684 (0x30), func_8004BF08, func_8005C144.
 *
 * func_80051510 already lives in func_80051980_port.c and is called, not
 * redefined.
 *
 * ARM DISCIPLINE. func_8005D2B4 is a 21-slot jump table: `jr $v0` after
 * `lw $v0,%lo(jtbl_800112DC)(at)`, with `addiu $a0,-0x44C` / `sltiu 0x15`.
 * The in-tree VM cannot execute a computed jump, so each arm is translated
 * explicitly. The five arms the game actually uses from `m0351i` module 1
 * (cmd 0x453 / 0x454 / 0x456 / 0x457 / 0x45E, decoded from the retail
 * package) are real ports. Every other arm reaches a subsystem that is not
 * translated yet; those arms are DOMAIN GUARDS: loud, one-shot, returning
 * retail's default-miss value 0. They are never a silent wrong answer and
 * are pinned by test_SEW26_arm_domain_guard.
 */
#include <stdio.h>
#include <string.h>
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GM_D_8009D254 0x8009D254u
#define GM_D_8009D300 0x8009D300u
#define GM_D_8009CE00 0x8009CE00u
#define GM_D_8009D2A4 0x8009D2A4u
#define GM_D_8009D154 0x8009D154u
#define GM_D_8009D1A0 0x8009D1A0u
#define GM_D_800B0CD8 0x800B0CD8u
#define GM_D_800C0E06 0x800C0E06u
#define GM_D_800C0E08 0x800C0E08u
#define GM_D_800C0E20 0x800C0E20u
#define GM_D_8009D02C 0x8009D02Cu
#define GM_D_800A1FD4 0x800A1FD4u
#define GM_D_800A1E6E 0x800A1E6Eu
#define GM_D_800A18B4 0x800A18B4u
#define GM_D_800A18D8 0x800A18D8u
#define GM_D_800A18FC 0x800A18FCu
#define GM_D_800A18FC_END 0x800A191Cu
#define GM_D_80092314 0x80092314u
#define GM_D_8009D2AC 0x8009D03Cu   /* gp+0x2CC: D_8005D39C window-id base */
#define GM_D_8009D01C 0x8009D01Cu   /* gp+0x2AC: func_8005270C fade target */
#define GM_D_8009CF80 0x8009CF80u   /* gp+0x210: "item-get node live" flag */
#define GM_D_8009CF84 0x8009CF84u   /* gp+0x214: message state (2 = closing) */

/* The five `m0351i` cmd constants (script operand, decoded from the retail
 * package m0351i module 1). */
#define GM_CMD_READ_C06  0x453
#define GM_CMD_SET_RECORD 0x454
#define GM_CMD_READ_MAX   0x456
#define GM_CMD_SET_LIMIT  0x457
#define GM_CMD_CLOSE      0x45E
#define GM_CMD_BASE       0x44C

/* Retail's arm 0x456 passes `$sp+0x10` to func_800515F8 as a nullable
 * out-pointer. The native port keeps that within-call temporary in guest
 * scratch instead of a host stack address (which is never a guest address).
 * This is a storage-location substitution only; the value written and read
 * back is exactly retail's. */
#define GM_TEMP_OUT (0x1F800380u)

static int gm_arm_guard_reported[21];

/* One-shot, loud guard for an arm whose callee is not translated yet. */
static int gm_arm_guard(unsigned index, unsigned cmd)
{
    if (index < 21u && !gm_arm_guard_reported[index]) {
        gm_arm_guard_reported[index] = 1;
        fprintf(stderr,
            "[MSG] func_8005D2B4 arm cmd=0x%03X reaches an unported "
            "field-message subsystem; returning retail default 0\n",
            cmd);
    }
    return 0;
}

/* ---- func_800515C0: message record +0xC / D_800C0E08 setter ------------- */
int func_800515C0(uint32_t value)
{
    pe_addr_t record = PE_LoadU32(GM_D_8009D254);
    if (record) {
        record = PE_LoadU32(record);
        if (record) PE_StoreU16(record + 0xCu, (uint16_t)value);
    }
    PE_StoreU16(GM_D_800C0E08, (uint16_t)value);
    return 0;
}

/* ---- func_80051684: message record +0x8 = value << 16 ------------------- */
int func_80051684(uint32_t value)
{
    pe_addr_t record = PE_LoadU32(GM_D_8009D254);
    if (record) {
        record = PE_LoadU32(record);
        if (record) PE_StoreU32(record + 8u, value << 16);
    }
    return 0;
}

/* ---- func_800629B0: "a message node is queued" flag --------------------- */
int func_800629B0(void)
{
    return PE_LoadU32(GM_D_8009D154) != 0u;
}

/* ---- func_80057E14: window-id remap table + func_80055760 --------------- */
int32_t func_80057E14(pe_addr_t list)
{
    int32_t count = 0;
    pe_addr_t out;
    uint32_t base, end;

    if (!list) {
        PE_StoreU32(0x8009D078u, 0u); /* gp+0x308 */
        return 0;
    }
    out = GM_D_800A1FD4;
    base = PE_LoadU32(GM_D_8009D2AC);
    end = base + 3u;
    while (count < 10) {
        int32_t id = (int16_t)PE_LoadU16(list);
        if (id == 0) break;
        if (id >= (int32_t)base && id < (int32_t)end) {
            int32_t remapped = id + 6 - (int32_t)base;
            PE_StoreU16(out, (uint16_t)(remapped + 0x200));
            PE_StoreU16(GM_D_800A1E6E + (uint32_t)remapped * 32u,
                        PE_LoadU16(list + 2u));
            out += 2u;
        } else {
            PE_StoreU16(out, (uint16_t)id);
            out += 2u;
        }
        count++;
        list += 4u;
    }
    PE_StoreU32(0x8009D04Cu, GM_D_800A1FD4);   /* gp+0x2DC */
    PE_StoreU32(0x8009D054u, (uint32_t)count); /* gp+0x2E4 */
    func_80055760();
    PE_StoreU32(0x8009D078u, (uint32_t)count); /* gp+0x308 */
    return count;
}

/* ---- func_8005270C: message window fade target -------------------------- */
int func_8005270C(void)
{
    pe_addr_t package = PE_LoadU32(0x800B0E08u);
    int32_t result = 0;
    if (package)
        result = func_8006DF50(package, 0x450u, 0x100u, 0x80u, 0x7Fu);
    PE_StoreU32(GM_D_8009D01C, (uint32_t)result); /* gp+0x2AC */
    return result;
}

/* ---- func_8004BF08: clear the two 8-word message tables ----------------- */
void func_8004BF08(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        PE_StoreU32(0x800A1940u + (uint32_t)i * 4u, 0u); /* retail stores this word first */
        PE_StoreU32(0x800A1920u + (uint32_t)i * 4u, 0u);
    }
}

/* ---- func_8004BE4C: build the "item get" window node pair --------------- */
void func_8004BE4C(void)
{
    pe_addr_t parent = func_80062D2C(0x15u, 0u, 0u, 0u);
    pe_addr_t child = func_8006322C(0x2Fu, parent, parent);

    PE_StoreU32(parent + 0x30u, 0x8004BF40u);
    PE_StoreU32(parent + 0x2Cu, 0x8004C1E0u);
    PE_StoreU32(child + 0x30u, 0x8004FFF8u);
    PE_StoreU32(child + 0x44u, 0xFFFFFFFFu);
    PE_StoreU32(child + 0x18u, PE_LoadU32(child + 0x18u) + 0x44u);
    PE_StoreU32(child + 0x40u, PE_LoadU32(child + 0x40u) - 2u);
    PE_StoreU32(child + 0x1Cu, PE_LoadU32(child + 0x1Cu) + 2u);
    func_80062CB8(parent);
    PE_StoreU32(parent + 0x4Cu, GM_D_80092314);
    PE_StoreU32(GM_D_8009CF80, 1u);   /* gp+0x210 */
    func_8004BF08();
}

/* ---- func_8005C144: snapshot the encounter style byte ------------------- */
void func_8005C144(void)
{
    PE_StoreU32(GM_D_8009D02C, func_80033A20() & 0xFFu);
    func_800339A0(0);
}

/* ---- func_8004BCE8: close/commit the active message window -------------- */
void func_8004BCE8(int32_t arg0)
{
    int32_t i;
    int32_t delta;
    pe_addr_t record;

    func_80051510();

    /* Publish the window geometry words. All nine stores target $gp offsets
     * 0x278/0x27C/0x1F0/0x280/0x1FC/0x1F4/0x200/0x1F8/0x204; gp is
     * 0x8009CD70, giving the guest literals below. */
    PE_StoreU32(0x8009CFE8u, PE_LoadU32(0x800C0E00u));   /* +0x278 */
    PE_StoreU32(0x8009CFECu, PE_LoadU32(0x800C0E00u));   /* +0x27C */
    PE_StoreU32(0x8009CF60u, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x1F0 */
    PE_StoreU32(0x8009CFF0u, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x280 */
    PE_StoreU32(0x8009CF6Cu, (uint32_t)PE_LoadU8(0x800C0E0Au)); /* +0x1FC */
    PE_StoreU32(0x8009CF64u, (uint32_t)PE_LoadU16(GM_D_800C0E06)); /* +0x1F4 */
    PE_StoreU32(0x8009CF70u, (uint32_t)PE_LoadU16(GM_D_800C0E06)); /* +0x200 */
    PE_StoreU32(0x8009CF68u, PE_LoadU32(0x800C0E10u));   /* +0x1F8 */
    delta = (int32_t)PE_LoadU32(0x800C0E10u) + (arg0 < 0 ? 0 : arg0);
    PE_StoreU32(0x8009CF74u, (uint32_t)delta);           /* +0x204 */

    /* Seven signed halfword keys -> func_8005B91C(i, key, &D_800A18B4[i], 0);
     * the key lands in D_800A18D8[i] and D_800A18FC[i] is cleared.  Retail
     * stores D_800A18D8[i] in the call's delay slot, so publish before the
     * call; the D_800A18FC[i] clear follows it. */
    {
        pe_addr_t key_src = 0x800C0E28u;
        for (i = 0; i < 7; i++) {
            int32_t key = (int16_t)PE_LoadU16(key_src + (uint32_t)i * 2u);
            PE_StoreU32(GM_D_800A18D8 + (uint32_t)i * 4u, (uint32_t)key);
            func_8005B91C(i, key, GM_D_800A18B4 + (uint32_t)i * 4u, 0u);
            PE_StoreU32(GM_D_800A18FC + (uint32_t)i * 4u, 0u);
        }
    }

    /* The scalar-temp `unsigned int *out` at $sp+0x10 lives in guest scratch,
     * since a host stack address is not a guest address. */
    func_8005B91C(0, (int32_t)PE_LoadU32(GM_D_800A18D8), GM_TEMP_OUT, 0u);
    PE_StoreU32(GM_D_8009CF84, 2u);   /* gp+0x214 */
    func_80057E14(0u);
    func_8004BE4C();
    record = func_80062A34(1u, 0x15u);
    func_80063158(record, 0, 0x20);
    if (arg0 > 0) func_8005270C();
    func_8005C144();
}

/* ---- func_8005D2B4: message-system query/mutate dispatcher -------------- */
int func_8005D2B4(int32_t cmd, int32_t a, int32_t b)
{
    unsigned index = (unsigned)(cmd - GM_CMD_BASE);

    if (index >= 21u) return 0;   /* retail `sltiu 0x15` miss */

    switch (index) {
    case 0x00: /* .L8005D2F0 — unported: cursor table reset/rebuild */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x01: /* .L8005D358 — unported: clamped accumulator */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x02: /* .L8005D39C — unported: glyph index lookup */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x03: /* .L8005D44C — unported: scroll pair store */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x04: /* .L8005D45C — unported: func_8005C688 subsystem */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x05: /* .L8005D46C — unported: clamp + table rebuild */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x06: /* .L8005D4B0 — unported: D_800C0E08 read */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x07: /* .L8005D4C0 — D_800C0E06 halfword read (cmd 0x453) */
        return (int32_t)PE_LoadU16(GM_D_800C0E06);
    case 0x08: /* .L8005D4D0 — func_800515C0(a) (cmd 0x454) */
        func_800515C0((uint32_t)a);
        return 0;
    case 0x09: /* .L8005D4E0 — unported: func_800515F8(NULL) */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x0A: /* .L8005D4F0 — func_800515F8(out); return *out (cmd 0x456) */
        func_800515F8(GM_TEMP_OUT);
        return (int32_t)PE_LoadU32(GM_TEMP_OUT);
    case 0x0B: /* .L8005D504 — func_80051684(a) (cmd 0x457) */
        func_80051684((uint32_t)a);
        return 0;
    case 0x0C: /* .L8005D514 — unported: signed index search */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x0D: /* .L8005D5B8 — unported: teardown sequence */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x0E: /* .L8005D5E8 — unported: func_80042CC4 */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x0F: /* .L8005D5F8 — unported: func_80042EDC */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x10: /* .L8005D608 — unported: func_80042F20 */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x11: /* .L8005D618 — unported: func_80053128 */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x12: /* .L8005D668 — func_8004BCE8(a) (cmd 0x45E) */
        func_8004BCE8(a);
        return 0;
    case 0x13: /* .L8005D678 — unported: short-table search/clear */
        return gm_arm_guard(index, (unsigned)cmd);
    case 0x14: /* .L8005D6D4 — unported: func_8005D020 */
        return gm_arm_guard(index, (unsigned)cmd);
    }
    return 0;
}

/* ---- func_80015C7C: field-VM opcode 0xE9 -------------------------------- */
int func_80015C7C(pe_addr_t args)
{
    pe_addr_t task = PE_LoadU32(GM_D_8009D300);
    pe_addr_t out  = PE_LoadU32(args + 0xCu);

    if ((PE_LoadU16(task + 8u) & 0x20u) == 0u)
        PE_StoreU32(out, (uint32_t)func_8005D2B4(
            (int32_t)PE_LoadU32(PE_LoadU32(args)),
            (int32_t)PE_LoadU32(PE_LoadU32(args + 4u)),
            (int32_t)PE_LoadU32(PE_LoadU32(args + 8u))));

    if (func_800629B0() != 0) {
        if ((PE_LoadU16(task + 8u) & 0x20u) == 0u) {
            PE_StoreU32(GM_D_800B0CD8, PE_LoadU32(GM_D_800B0CD8) | 0x9000u);
            PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) | 0x20u));
            func_80067CBC();
        } else {
            PE_StoreU32(GM_D_8009D1A0, PE_LoadU32(GM_D_8009D1A0) | 4u);
        }
        PE_StoreU32(GM_D_8009CE00, PE_LoadU32(GM_D_8009CE00) - 0x1Cu);
        PE_StoreU32(task + 0x10u, 1u);
        return 0;
    }

    if ((int16_t)PE_LoadU16(GM_D_8009D2A4) != 0) {
        PE_StoreU32(out, (uint32_t)(int32_t)(int16_t)PE_LoadU16(GM_D_8009D2A4));
        PE_StoreU16(task + 8u, (uint16_t)(PE_LoadU16(task + 8u) & 0xFFDFu));
    }
    return 1;
}
