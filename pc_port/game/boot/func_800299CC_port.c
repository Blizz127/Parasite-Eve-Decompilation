/*
 * PE-CH1 — func_800299CC_consume_cut: BTL1 D_8009D28C 6→0 consume
 * (named cut of the battle/field tick; not the 456-byte dispatcher).
 *
 * Full retail body starts at 0x800299CC (addiu $sp, -456) and runs
 * far past this cut. This translation is ONLY the consume-edge prefix:
 *
 *   0x800299CC..0x80029A0C exclusive (16 words / 0x40, file 0x1A1CC).
 *   First excluded word: lw $v0, 0x4C($a0) at 0x80029A0C (reload of
 *   the same flag word; not a consume store).
 *
 * Words dumped from SHA-1-exact build/disc1.candidate.exe
 * (452fb033f2eaa4b18aa20a5bca60b8125af3a37b). yaml [0x11718, asm]
 * (before matching 2F970 @ file 0x20170). This worktree has no
 * era/asm split, so the body cannot yet be a matching src/ unit.
 *
 * ROM (gp = 0x8009CD70):
 *
 *   lw    a0, 0x508($gp)          D_8009D278 current record
 *   addiu sp, -456                (frame; not guest-observable here)
 *   ... save s1/ra/s0 ...
 *   lw    v0, 0x4C(a0)
 *   lui   a1, 0x8                 0x00080000
 *   and   v0, a1
 *   beqz  v0, 0x80029A20          skip consume (past exclusive end)
 *   addiu s1, 1                   delay (register only)
 *   lw    v1, 0x51C($gp)          D_8009D28C
 *   addiu v0, 6
 *   bne   v1, v0, 0x80029A0C      skip consume (lands on exclusive end)
 *   addiu v0, 6                   delay: value stored by sb
 *   sb    v0, 0x10C($gp)          D_8009CE7C = 6
 *   sw    zero, 0x51C($gp)        D_8009D28C = 0
 *
 * Both guards required: record+0x4C & 0x00080000, and mode == 6.
 * Skip paths in this cut leave mode and gp+0x10C untouched.
 *
 * Inverse handshake at 0x80029A20 (lbu gp+0x10C==6 → sb 0 / restore
 * mode 6) is AFTER the exclusive end; not this cut. Later dispatcher
 * stores (Aya reload, mode!=0 branch @0x80029A64) are out of scope.
 *
 * Sole jal of the FULL tick: 0x800355E8. Opcode 0x89 matching leaf
 * src/func_80017FF0.c writes 6; opcode 0x94 native func_80019154
 * reads it. This cut is the consumer.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D278 0x8009D278u /* gp+0x508 current record */
#define GA_D_8009D28C 0x8009D28Cu /* gp+0x51C mode word */
#define GA_D_8009CE7C 0x8009CE7Cu /* gp+0x10C consume-edge byte */
#define RECORD_FLAG   0x00080000u

void func_800299CC_consume_cut(void)
{
    pe_addr_t record;
    unsigned int flags;

    record = PE_LoadU32(GA_D_8009D278);
    flags = PE_LoadU32(record + 0x4Cu);
    if ((flags & RECORD_FLAG) == 0u) {
        return;
    }
    if (PE_LoadU32(GA_D_8009D28C) != 6u) {
        return;
    }
    PE_StoreU8(GA_D_8009CE7C, 6u);
    PE_StoreU32(GA_D_8009D28C, 0u);
}

/*
 * PE-BTL53 — 299CC after the consume exclusive end, through the
 * 4D4 idle gate. Not the 2A470/1D340 clip body.
 *
 *   0x80029A0C  reload record+0x4C & 0x00080000
 *   0x80029A20  inverse: flag clear && edge==6 → edge=0, mode=6
 *   0x80029A38  *D254 → D278; sw 0 → D_8009D230
 *   0x80029A5C  jal 5C498 not this cut
 *   0x80029A6C  mode!=0 → 2A7F8 (mode switch not this cut)
 *   0x80029A7C  4D4==0 → 2A7F8
 *
 * Live after 293F4(0): 4D4 is 0, so 1D340/27D14 (rec=4) are
 * not reached. Do not invent 4D4 or rec=4.
 */
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D230 0x8009D230u
#define GA_D_8009D244 0x8009D244u /* gp+0x4D4 */

void func_800299CC_after_consume_cut(void)
{
    pe_addr_t record;
    pe_addr_t aya;
    unsigned int flags;

    record = PE_LoadU32(GA_D_8009D278);
    flags = PE_LoadU32(record + 0x4Cu);
    if ((flags & RECORD_FLAG) == 0u) {
        if (PE_LoadU8(GA_D_8009CE7C) == 6u) {
            PE_StoreU8(GA_D_8009CE7C, 0u);
            PE_StoreU32(GA_D_8009D28C, 6u);
        }
    }

    aya = PE_LoadU32(GA_D_8009D254);
    /* Kuseg *D254 is the KSEG0 word (BTL-RAM-LOW). */
    if (aya < 0x80000000u)
        aya |= 0x80000000u;
    PE_StoreU32(GA_D_8009D230, 0u);
    PE_StoreU32(GA_D_8009D278, PE_LoadU32(aya));

    if (PE_LoadU32(GA_D_8009D28C) != 0u)
        return;
    if (PE_LoadU8(GA_D_8009D244) == 0u)
        return;
}

/*
 * PE-BTL97 — 2A7F8 mode==6 arm jals 2BC90 (0x8002A9EC).
 * 2BC90 prefix: jal 21D4C / 374E8 not this cut. Retail delay
 * at 2BCA8 sets s1=1, then D2E8|=1, *D254+0x68/6C/70=0,
 * D1A0&=~4. Actor walk 2CD40..2CEDC is not this cut.
 * 2CEE0 tail (fall-through at 2CED8 with s1 still 1):
 * jal 6914C(0); v0==0 → 2CF24 mode 7. Do not store 4D4.
 */
#define GA_D_8009D2E8 0x8009D2E8u

void func_8002BC90_mode6_cut(void)
{
    pe_addr_t aya;

    PE_StoreU32(GA_D_8009D2E8, PE_LoadU32(GA_D_8009D2E8) | 1u);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u) {
        if (aya < 0x80000000u)
            aya |= 0x80000000u;
        PE_StoreU32(aya + 0x68u, 0u);
        PE_StoreU32(aya + 0x6Cu, 0u);
        PE_StoreU32(aya + 0x70u, 0u);
    }
    D_8009D1A0 &= ~4u;
    if (func_8006914C(0) == 0)
        func_8002CF24_mode7_cut();
}

void func_800299CC_mode_switch_cut(void)
{
    uint32_t mode;

    mode = PE_LoadU32(GA_D_8009D28C);
    if (mode == 6u)
        func_8002BC90_mode6_cut();
    else if (mode == 3u)
        func_8002A7F8_mode3_cut();
}

/*
 * PE-BTL98 — 299CC mode==0 && 4D4!=0 body reaches jal 1D340
 * at 0x8002A4FC. s1 is 1 from the consume delay at 0x800299F0
 * unless 2A444/2A4BC overwrite it (mode-1 / bit-0x4000 arms,
 * not first retail entry). a0 = s1. Do not store 4D4 or HP.
 */
void func_800299CC_damage_entry_cut(void)
{
    if (PE_LoadU32(GA_D_8009D28C) != 0u)
        return;
    if (PE_LoadU8(GA_D_8009D244) == 0u)
        return;
    func_8001D340(1u);
}
