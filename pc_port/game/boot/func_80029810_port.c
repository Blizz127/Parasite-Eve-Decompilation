/*
 * PE-BTL3 — ROM-word cuts from func_80029810, func_8001A680, and the
 * type-0 command-table writer at 0x8006C140 (func_8006BECC state 6).
 *
 * These are native translations, not matching src/ C.  Authority is
 * pc_port/tools/pe_btl3_29810_tail_oracle.py and
 * pc_port/tools/pe_btl3_command_table_oracle.py against the SHA-1-exact EXE.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_RECORD_P       0x8009D278u
#define GA_ACTOR_P        0x8009D254u
#define GA_COMMAND_TABLE  0x800B0E98u
#define GA_VALUE_D27C     0x8009D27Cu
#define GA_OVERLAY        0x800B0CD8u
#define CALLBACK_2D268    0x8002D268u

/*
 * ROM 0x8006C140..0x8006C174 (Writer B clip loop inside func_8006BECC
 * state 6). s4 = D_800B0CD8, s2 = overlay+0x154 package. Each 12-byte
 * directory record stores package+(rec+4 & 0x00FFFFFF) at
 * overlay+0x1C0+idB*4 = D_800B0E98[idB] (type-0 row). Directory setup
 * matches 0x8006C0E4/0xEC/0xF4 and 0x8006C11C..0x130 (section at
 * package+4, count = packed>>22, records at packed&0x3FFFFF). Model
 * bind 0x8006C118 (D_800B0E70) is not this cut.
 *
 * 0x55 / func_80029810 does not jal this writer. The table is a global
 * filled by 6BECC; 1A680 only reads it. Native tests invoke the writer
 * with a planted package so the consumer can use a ROM-shaped pointer.
 */
void func_8006C140_type0_clip_bind(pe_addr_t package)
{
    pe_addr_t dir;
    unsigned int packed;
    unsigned int count;
    unsigned int i;
    pe_addr_t rec;

    dir = package + PE_LoadU32(package + 4u);
    packed = PE_LoadU32(dir + 0x10u);
    count = packed >> 22;
    rec = package + (packed & 0x3FFFFFu);
    for (i = 0; i < count; i++) {
        unsigned int idb = PE_LoadU8(rec + 7u);
        unsigned int ptr = PE_LoadU32(rec + 4u) & 0x00FFFFFFu;
        PE_StoreU32(GA_OVERLAY + 0x1C0u + idb * 4u, package + ptr);
        rec += 12u;
    }
}

void func_8001A680_command_cut(pe_addr_t actor, unsigned int command)
{
    unsigned int type;
    pe_addr_t resource;
    unsigned int flags;

    type = PE_LoadU8(actor + 0x0Cu);
    resource = PE_LoadU32(GA_COMMAND_TABLE + type * 192u
                          + (command & 0xFFFFu) * 4u);
    PE_StoreU8(actor + 0x0Eu, (uint8_t)command);
    PE_StoreU32(actor + 0x14u, 0u);
    PE_StoreU32(actor + 0x18u, 0u);
    PE_StoreU32(actor + 0x1B0u, resource);
    flags = PE_LoadU32(actor + 0x98u) & ~0x200u;
    PE_StoreU32(actor + 0x98u, flags);
    PE_StoreU8(actor + 0x0Fu,
               (uint8_t)((PE_LoadU8(resource + 2u) - 1u) & 0xFFu));
}

/*
 * PE-BTL66 — func_8001A4AC clip ticker.
 *
 * 117 words 0x8001A4AC..0x8001A680, SHA-256 53d93b57…2174.
 * Callers: self @ 1A4E4, 35558 @ 35B84 (D254) and 35BEC (D20C).
 * Jals 6A318 and 1A784 only when actor+0x18C != 0; 125E0 zeros
 * that pointer, so those callees are not this cut.
 *
 * Clears +0x98 bit 3, sets bit 0x800000, copies +0x14 → +0x18.
 * +0x1A is the high half of that copy, so it lags +0x16 by one
 * tick. 2B0E8 phase 2 and 24A3C case 9 wait +0x0F==+0x1A.
 * Bit 0x100 returns. Bit 0x200 and +0x16==+0x12 returns.
 * Else +0x14 += +0x1C (35038 stores 0x10000) and clamps to
 * +0x12<<16. +0x16 is the high half of +0x14: 2B0E8 phase 0
 * waits that half == 10. +0x0F wrap / bit 8 is the no-0x200
 * path.
 */
void func_8001A4AC(pe_addr_t actor)
{
    pe_addr_t child;
    uint32_t flags;
    uint32_t cur;
    uint32_t speed;
    uint32_t next;
    uint32_t target;
    uint32_t dest;
    uint32_t cap;
    int32_t scur;
    int32_t snext;
    int32_t starget;

    if (actor == 0u)
        return;
    child = PE_LoadU32(actor + 0x18Cu);
    if (child != 0u) {
        if ((PE_LoadU32(child + 0x98u) & 0x800000u) == 0u)
            func_8001A4AC(child);
        /* jal 6A318 not this cut. */
    }
    flags = (PE_LoadU32(actor + 0x98u) & ~8u) | 0x800000u;
    PE_StoreU32(actor + 0x98u, flags);
    cur = PE_LoadU32(actor + 0x14u);
    PE_StoreU32(actor + 0x18u, cur);
    if (flags & 0x100u)
        return;
    dest = PE_LoadU16(actor + 0x12u);
    if ((flags & 0x200u) != 0u && (cur >> 16) == dest)
        return;
    if (flags & 0x200000u) {
        /* jal 1A784; copy child+0x14. Not this cut. */
        return;
    }
    speed = PE_LoadU32(actor + 0x1Cu);
    next = cur + speed;
    target = dest << 16;
    cap = PE_LoadU8(actor + 0x0Fu);
    if ((int32_t)next >> 16 > (int32_t)cap) {
        uint32_t denom;

        denom = cap + 1u;
        if (denom == 0u)
            return;
        next = (((int32_t)next >> 16) / (int32_t)denom) << 16;
        cur = 0u;
        flags |= 8u;
        PE_StoreU32(actor + 0x98u, flags);
    } else if ((int32_t)next < 0) {
        next += (cap + 1u) << 16;
        flags |= 8u;
        PE_StoreU32(actor + 0x98u, flags);
    }
    flags = PE_LoadU32(actor + 0x98u);
    if ((flags & 0x200u) == 0u) {
        PE_StoreU32(actor + 0x14u, next);
        return;
    }
    scur = (int32_t)cur;
    snext = (int32_t)next;
    starget = (int32_t)target;
    if ((scur < starget && starget < snext)
        || (starget < scur && snext < starget))
        PE_StoreU32(actor + 0x14u, target);
    else
        PE_StoreU32(actor + 0x14u, next);
}

/*
 * R3000 Kuseg/KSEG0 2 MiB RAM mirror. 209F0 has no null check on
 * D278+0x68. After 29810, D278 is *D254 = slot body (0x6F / 2F7D8
 * copies D_800109B0). That template +0x68 is 0; TEXT has no non-zero
 * sw to D278+0x68 or *D254+0x68. Actor+0x68 is a different field
 * (35038 / 35558 motion; opcode D2F0 stores).
 *
 * APPROXIMATION: bytes at physical 0+6 are not in the EXE. Host
 * guest RAM is zero there. Replace when a post-boot RAM image of
 * the exception-vector area is recovered. Not an invented pointer.
 */
static pe_addr_t pe_kseg0(pe_addr_t addr)
{
    return 0x80000000u | (addr & 0x1FFFFFu);
}

/* EXE 0x800106A4 (9×5) and 0x800106D4 (9). */
static const uint8_t k_209f0_row[45] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0A,
    0x02, 0x05, 0x00, 0x00, 0x0A, 0x04, 0x00, 0x00, 0x00,
    0x0A, 0x02, 0x0A, 0x07, 0x00, 0x0A, 0x04, 0x00, 0x00,
    0x05, 0x0F, 0x03, 0x00, 0x00, 0x14, 0x1E, 0x03, 0x00,
    0x00, 0x05, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05
};
static const uint8_t k_209f0_scale[9] = {
    0x00, 0x0A, 0x08, 0x0A, 0x08, 0x08, 0x04, 0x0A, 0x14
};

/*
 * 209F0. D278 sb +0x12=4 .. +0x19=11, sh +0x24 = (lhu+0x22 *
 * scale[lbu(obj+6)]) / 10, copy 5-byte row to obj+0x14.. and
 * sw row[4] to obj+8, jal 6C4C4(lh(obj+6)). obj = lw(D278+0x68)
 * through Kuseg. Slot-body +0x68 is 0 (109B0). 6C4C4 a0 is
 * lh(obj+6). APPROXIMATION: host RAM[6]==0. D1A0 bit1 still
 * sets +0xE bit1 regardless of a0 (ROM).
 */
void func_800209F0_cut(void)
{
    pe_addr_t record;
    pe_addr_t obj;
    unsigned int idx;
    unsigned int scale;
    int prod;
    int hi;
    const uint8_t *row;

    record = PE_LoadU32(GA_RECORD_P);
    obj = pe_kseg0(PE_LoadU32(record + 0x68u));
    idx = PE_LoadU8(obj + 6u);
    scale = (idx < 9u) ? k_209f0_scale[idx] : 0u;
    row = (idx < 9u) ? &k_209f0_row[idx * 5u] : k_209f0_row;
    prod = (int)PE_LoadU16(record + 0x22u) * (int)scale;
    hi = (int)(((long long)prod * 0x66666667LL) >> 32);
    PE_StoreU8(record + 0x12u, 4u);
    PE_StoreU8(record + 0x13u, 5u);
    PE_StoreU8(record + 0x14u, 6u);
    PE_StoreU8(record + 0x17u, 7u);
    PE_StoreU8(record + 0x15u, 8u);
    PE_StoreU8(record + 0x18u, 9u);
    PE_StoreU8(record + 0x16u, 10u);
    PE_StoreU8(record + 0x19u, 11u);
    PE_StoreU16(record + 0x24u, (uint16_t)(hi >> 2));
    PE_StoreU8(obj + 0x14u, row[0]);
    PE_StoreU8(obj + 0x15u, row[1]);
    PE_StoreU8(obj + 0x16u, row[2]);
    PE_StoreU8(obj + 0x17u, row[3]);
    PE_StoreU32(obj + 8u, row[4]);
    (void)func_8006C4C4((int)(int16_t)PE_LoadU16(obj + 6u));
}

/*
 * 30640. lw D278+0x68, lw obj+0x10, and 0x10000. Slot-body
 * +0x68=0; APPROXIMATION host RAM+0x10=0 skips 71A54. That arm
 * is not invented.
 */
void func_80030640_cut(void)
{
    pe_addr_t record;
    pe_addr_t obj;

    record = PE_LoadU32(GA_RECORD_P);
    obj = pe_kseg0(PE_LoadU32(record + 0x68u));
    if ((PE_LoadU32(obj + 0x10u) & 0x00010000u) == 0u)
        return;
    Bootstrap_ReturnVoid("func_80071A54", "func_80030640_cut");
}

/*
 * 339A0 32w. Copy 16B 0x80010E38 onto $sp, index (a0&0xFF)*4,
 * sh pair to D_8009CE84 / D_8009CE86, sb a0 to D_8009CE80.
 * 0 jals. Not ATB/AI/damage.
 */
void func_800339A0_cut(unsigned int encounter)
{
    static const uint16_t k_pairs[4][2] = {
        { 0x00DDu, 0x00B1u },
        { 0x000Fu, 0x00B1u },
        { 0x000Fu, 0x000Fu },
        { 0x00DDu, 0x000Fu },
    };
    unsigned int idx;

    idx = encounter & 0xFFu;
    PE_StoreU8(0x8009CE80u, (uint8_t)idx);
    if (idx < 4u) {
        PE_StoreU16(0x8009CE84u, k_pairs[idx][0]);
        PE_StoreU16(0x8009CE86u, k_pairs[idx][1]);
    }
}

void func_80029810_after_hp_cut(unsigned int encounter)
{
    pe_addr_t record;
    pe_addr_t actor;
    pe_addr_t source;
    int value;
    int cap;

    func_800209F0_cut();
    record = PE_LoadU32(GA_RECORD_P);
    value = (int)PE_LoadU32(record + 0x08u);
    if (value <= 0) {
        PE_StoreU32(record + 0x08u, 0x00010000u);
    } else {
        cap = (int)PE_LoadU32(record + 0x28u);
        if (value >= cap) {
            PE_StoreU32(record + 0x08u, (unsigned int)cap);
            PE_StoreU32(record + 0x34u, 240u);
        }
    }

    func_80030640_cut();
    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(actor + 0x194u, CALLBACK_2D268);
    source = PE_LoadU32(actor + 0x238u);
    PE_StoreU16(GA_VALUE_D27C,
                (uint16_t)(PE_LoadU32(source + 0x18u) - 100u));
    func_800339A0_cut(encounter);
    record = PE_LoadU32(GA_RECORD_P);
    func_8001A680_command_cut(actor, PE_LoadU8(record + 0x12u));
}

/*
 * 144FC 0x3B at 0x80014630 (12 words to park). $s0 = overlay
 * D_800B0CD8. Busy (+0xE&3) j 0x80014660 v0=0. Clear: lw/sw
 * 0($s0) &= ~0x00800000, sb F4=0, j 0x8001467C v0=1.
 * 0x3A's $s1 binder is a different object (8E22 vs 8E02).
 * No jal/jalr; not 6914C; not 0x800E086C. v0=1 is not issued
 * on the live park (bits stay set until 3F074's 6C5BC poll
 * returns 0 via EE=0 → 6CC68 after 6CC2C).
 */
int func_800144FC_state3B_cut(void)
{
    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) != 0u)
        return 0;
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0u);
    PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) & 0xFF7FFFFFu);
    return 1;
}

/*
 * 144FC state 0 at 0x80014544. If (+0xE & 3)==0, sb 0x37 → +0xF4;
 * always overlay[0] |= 0x00800000 (lw/sw 0($s0)). j 0x80014660
 * v0=0. Does not complete 0x55.
 */
int func_800144FC_state0_cut(void)
{
    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) == 0u)
        PE_StoreU8(GA_OVERLAY + 0xF4u, 0x37u);
    PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) | 0x00800000u);
    return 0;
}

/*
 * 0x80042EDC..0x80042F20 exclusive (17 words). lbu D_800BD024,
 * sw 1 → gp+0x168 / gp+0x174, sw 0 → gp+0x178, clamp the byte
 * into gp+0x16C: <0 → 1 (dead after lbu), >=33 → 32. Two TEXT
 * jals: 144FC 0x37 @ 0x80014588 and 0x8005D5F8.
 */
void func_80042EDC(void)
{
    unsigned int value;

    value = PE_LoadU8(0x800BD024u);
    PE_StoreU32(0x8009CED8u, 1u);
    PE_StoreU32(0x8009CEE4u, 1u);
    PE_StoreU32(0x8009CEE8u, 0u);
    if ((int)value < 0)
        PE_StoreU32(0x8009CEDCu, 1u);
    else if (value >= 33u)
        PE_StoreU32(0x8009CEDCu, 32u);
    else
        PE_StoreU32(0x8009CEDCu, value);
}

/*
 * 0x80042F20..0x80042F38 exclusive (6 words). Twin of 42EDC's two
 * gp stores only: addiu 5 / sw gp+0x168, addiu -1 / sw gp+0x174,
 * jr+nop. Next leaf at 0x80042F38 is a different function
 * (sw $zero, gp+0x168). Two TEXT jals: 144FC 0x38 @ 0x800145C8
 * and 0x8005D608.
 */
void func_80042F20(void)
{
    PE_StoreU32(0x8009CED8u, 5u);
    PE_StoreU32(0x8009CEE4u, 0xFFFFFFFFu);
}

/*
 * 144FC state 0x37 at 0x80014570. If overlay word bit 0x400000 is
 * clear, jal 42EDC; always sb 0x38 → +0xF4 and re-dispatch
 * (j 0x80014518). Named cut does not enter 0x38 / 6D60C.
 */
int func_800144FC_state37_cut(void)
{
    if ((PE_LoadU32(GA_OVERLAY) & 0x00400000u) == 0u)
        func_80042EDC();
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x38u);
    return 0;
}

extern void func_80087024(void);

#define GA_GP_418 0x8009D188u
#define GA_GP_41C 0x8009D18Cu
#define GA_GP_420 0x8009D190u
#define GA_CDA4   0x8009CDA4u

/*
 * 6D60C after 6D078 returns 0, at 0x8006D79C. Overlay word bit 0x4
 * (set by 0x2C) and bit 0x40 select the arm. Live NYPD: bit4 set,
 * bit40 clear → sb F2=0x3F, re-dispatch. 0x3F with the same bits
 * sb 0x2F. 0x2F jals 6CDA4(0, lb +0xE1, …); a0==0 parks (87198
 * not stubbed). 86C5C / 6DF50 / 864CC are not stubbed.
 */
void func_8006D60C_after_6d078_cut(void)
{
    unsigned int word;
    int remain;

    word = PE_LoadU32(GA_OVERLAY);
    if ((word & 4u) != 0u && (word & 0x40u) != 0u) {
        remain = 60 - ((int)PE_LoadU32(GA_CDA4) - (int)PE_LoadU32(GA_GP_41C));
        PE_StoreU32(GA_GP_420, (unsigned int)remain);
        if (remain >= 9)
            PE_StoreU32(GA_GP_420, 8u);
        else if (remain < 0)
            PE_StoreU32(GA_GP_420, 0u);
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x3Fu);
}

int func_8006D60C_state3F_cut(void)
{
    unsigned int word;
    int timer;

    word = PE_LoadU32(GA_OVERLAY);
    if ((word & 4u) == 0u) {
        PE_StoreU8(GA_OVERLAY + 0xF2u, 0x30u);
        return 1;
    }
    if ((word & 0x40u) == 0u) {
        PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Fu);
        return 1;
    }
    timer = (int)PE_LoadU32(GA_GP_420);
    if (timer > 0) {
        PE_StoreU32(GA_GP_420, (unsigned int)(timer - 1));
        return 1;
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Fu);
    return 1;
}

int func_8006D60C_state2F_cut(void)
{
    if (func_8006CDA4(0, (int)(int8_t)PE_LoadU8(GA_OVERLAY + 0xE1u), 0,
                      PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x30u);
    return 0;
}

int func_8006D60C_state30_cut(void)
{
    func_80086464(PE_LoadU32(GA_OVERLAY + 0x124u));
    func_80086C1C(0, 0x7F);
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0u);
    return 0;
}

void func_8006D60C_state0_a0eq1_cut(void)
{
    PE_StoreU32(GA_GP_418, 0u);
    func_80087024();
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Cu);
}

void func_8006D60C_state2C_cut(void)
{
    if ((int8_t)PE_LoadU8(GA_OVERLAY + 0xE0u)
        != (int8_t)PE_LoadU8(GA_OVERLAY + 0xDCu)) {
        PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) | 4u);
    }
    PE_StoreU8(GA_OVERLAY + 0xF2u, 0x2Eu);
}

int func_8006D60C(int a0)
{
    unsigned int f2;
    unsigned int hops;

    f2 = PE_LoadU8(GA_OVERLAY + 0xF2u);
    if (f2 >= 0x41u)
        return 0;
    for (hops = 0; hops < 16u; hops++) {
        if (f2 == 0u) {
            if (a0 == 0)
                return 1;
            func_8006D60C_state0_a0eq1_cut();
            f2 = 0x2Cu;
            continue;
        }
        if (f2 == 0x2Cu) {
            func_8006D60C_state2C_cut();
            f2 = 0x2Eu;
            continue;
        }
        if (f2 == 0x2Eu) {
            if (func_8006D078() == 1)
                return 1;
            func_8006D60C_after_6d078_cut();
            f2 = PE_LoadU8(GA_OVERLAY + 0xF2u);
            continue;
        }
        if (f2 == 0x3Fu) {
            (void)func_8006D60C_state3F_cut();
            f2 = PE_LoadU8(GA_OVERLAY + 0xF2u);
            if (f2 == 0x3Fu)
                return 1;
            continue;
        }
        if (f2 == 0x2Fu) {
            if (func_8006D60C_state2F_cut() == 1)
                return 1;
            f2 = PE_LoadU8(GA_OVERLAY + 0xF2u);
            continue;
        }
        if (f2 == 0x30u)
            return func_8006D60C_state30_cut();
        return 0;
    }
    return 1;
}

int func_800144FC_state38_cut(void)
{
    if (func_8006D60C(1) == 1)
        return 0;
    if ((PE_LoadU32(GA_OVERLAY) & 0x00400000u) == 0u)
        func_80042F20();
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x39u);
    return 0;
}

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);
extern pe_addr_t func_8006E498(pe_addr_t base, uint32_t key);
extern unsigned int D_8009D1A0;

#define GA_930E2  0x800930E2u
#define GA_930E4  0x800930E4u
#define GA_942E4  0x800942E4u
#define GA_942E8  0x800942E8u

/*
 * 6914C state 0 table prefix at 0x800691D0. Two 11-slot inits:
 * dest=+0x188 stride 0xA0C, then dest=+0x188+0x6E84 stride 0x10C.
 * Each slot: sb 0, sb 0xFF ×3, sw 0 at +4, sw 0 at +8. 6A8D4
 * publishes +0x188=D_800B0E60. jalr D_800942E0 → 0x800E0xxx
 * overlay leaves are not invented (overlay not loaded).
 */
static void func_8006914C_state0_tables(void)
{
    pe_addr_t base;
    pe_addr_t slot;
    unsigned int i;

    base = PE_LoadU32(GA_OVERLAY + 0x188u);
    PE_StoreU32(GA_942E4, base);
    if (base == 0u)
        return;
    for (i = 0; i < 11u; i++) {
        slot = base + i * 0xA0Cu;
        PE_StoreU8(slot, 0u);
        PE_StoreU8(slot + 1u, 0xFFu);
        PE_StoreU8(slot + 2u, 0xFFu);
        PE_StoreU8(slot + 3u, 0xFFu);
        PE_StoreU32(slot + 4u, 0u);
        PE_StoreU32(slot + 8u, 0u);
    }
    base = PE_LoadU32(GA_OVERLAY + 0x188u) + 0x6E84u;
    PE_StoreU32(GA_942E8, base);
    for (i = 0; i < 11u; i++) {
        slot = base + i * 0x10Cu;
        PE_StoreU8(slot, 0u);
        PE_StoreU8(slot + 1u, 0xFFu);
        PE_StoreU8(slot + 2u, 0xFFu);
        PE_StoreU8(slot + 3u, 0xFFu);
        PE_StoreU32(slot + 4u, 0u);
        PE_StoreU32(slot + 8u, 0u);
    }
}

/*
 * func_8006914C is 274 words (0x8006914C..0x80069594). Dispatch on
 * lbu +0xEF. State 0: if D1A0 bit 0x80 clear, fill the two 11-slot
 * tables then jalr D_800942E0 (overlay 0x800E0xxx — not invented).
 * Proven tail: overlay |= 8, D1A0 |= 0x80. a0!=0 && bit 8 → sb 0x34,
 * return 1. 0x34 jals real 6E6A8(LBA=+0x100+lhu 930E2,
 * dest=+0x194=6A8D4 D_800B0E6C=0x801ED800, sectors=5). -1 stays
 * 0x34 return 1; else sb 0x35 return 1. 0x35 jals real 6E7E8;
 * host-sync → sb 0x36 return 1. 0x36 jals 6E1C0 × count (LoadImage
 * over the 10240B TIM-like dest; does not write it) then 6E498
 * (+0x18C, key 0x73DECD80). Done: sb EF=0, overlay&=~8, v0=0.
 * Next 6914C(1) at EF=0 with bit8 clear returns 0 (not a stub).
 * a0==0 at 0x800693B4: D1A0 bit 1 clear → sb EF=0, v0=0.
 * a0==0 and bit 1 set uses the same overlay&8 arm as a0!=0.
 * 2CEE0 needs that v0=0 (plus s1!=0) to reach 2CF24. Does not sb 0x3A.
 */
int func_8006914C(int a0)
{
    unsigned int ef;
    unsigned int word;
    int status;
    unsigned int off;
    unsigned int end;

    ef = PE_LoadU8(GA_OVERLAY + 0xEFu);
    if (ef == 0u) {
        if ((D_8009D1A0 & 0x80u) == 0u) {
            func_8006914C_state0_tables();
            word = PE_LoadU32(GA_OVERLAY);
            PE_StoreU32(GA_OVERLAY, word | 8u);
            D_8009D1A0 |= 0x80u;
        }
        if (a0 == 0 && (D_8009D1A0 & 2u) == 0u) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
            return 0;
        }
        if ((PE_LoadU32(GA_OVERLAY) & 8u) != 0u) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x34u);
            return 1;
        }
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
        return 0;
    }
    if (ef == 0x34u) {
        off = PE_LoadU16(GA_930E2);
        end = PE_LoadU16(GA_930E4);
        status = func_8006E6A8(
            (int)(PE_LoadU32(GA_OVERLAY + 0x100u) + off),
            PE_LoadU32(GA_OVERLAY + 0x194u),
            (int)(end - off));
        if (status != -1)
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x35u);
        return 1;
    }
    if (ef == 0x35u) {
        status = func_8006E7E8();
        if (status == -1) {
            PE_StoreU8(GA_OVERLAY + 0xEFu, 0x34u);
            return 1;
        }
        if (status != 0)
            return 1;
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0x36u);
        return 1;
    }
    if (ef == 0x36u) {
        pe_addr_t dest;
        pe_addr_t block;
        pe_addr_t entry;
        pe_addr_t arch;
        pe_addr_t found;
        unsigned int word;
        int count;
        int i;
        uint32_t key;
        RECT rc;
        uint32_t dims;
        uint32_t h;
        uint32_t off;

        dest = PE_LoadU32(GA_OVERLAY + 0x194u);
        block = dest + PE_LoadU32(dest + 4u);
        word = PE_LoadU32(block + 0x28u);
        count = (int)(word >> 16);
        if (count > 0) {
            entry = dest + (word & 0xFFFFu);
            for (i = 0; i < count; i++) {
                (void)func_8006E1C0(entry, dest);
                entry += 0x14u;
            }
        }
        arch = PE_LoadU32(GA_OVERLAY + 0x18Cu);
        key = 0x73DECD80u;
        while (arch != 0u) {
            found = func_8006E498(arch, key);
            if (found == 0u)
                break;
            dims = PE_LoadU32(found + 8u);
            rc.x = (int16_t)((dims >> 10) & 0x7FFu);
            rc.y = (int16_t)(dims >> 21);
            rc.w = (int16_t)(dims & 0x3FFu);
            h = PE_LoadU8(found + 7u);
            if (h == 0u)
                h = 0x100u;
            else
                h &= 0xFFu;
            rc.h = (int16_t)h;
            off = PE_LoadU32(found + 4u) & 0x00FFFFFFu;
            func_8007506C(&rc, found + off);
            key += 4u;
        }
        PE_StoreU8(GA_OVERLAY + 0xEFu, 0u);
        PE_StoreU32(GA_OVERLAY, PE_LoadU32(GA_OVERLAY) & 0xFFFFFFF7u);
        return 0;
    }
    return 1;
}

/*
 * 144FC state 0x39 at 0x800145DC. jal 6914C(1); v0==1 parks at
 * 0x80014660 (v0=0). Else sb 0x3A and re-dispatch. Named cut does
 * not enter 0x3A / 29810 / mode 7.
 */
int func_800144FC_state39_cut(void)
{
    if (func_8006914C(1) == 1)
        return 0;
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x3Au);
    return 0;
}

/*
 * BIOS A(0x30) std_out_puts trampoline (3 words at 0x80071A64).
 * 29810 passes lw D_8009D250. Live boot value is 0; puts(0) has no
 * guest stores. Nonzero a0 stays an honest boundary — no invented
 * console/battle state.
 */
void func_80071A64(pe_addr_t str)
{
    if (str == 0u)
        return;
    Bootstrap_ReturnVoid1("func_80071A64", "func_80029810", str);
}

/*
 * 29810 remainder 0x80029854..0x800298FF (43 words). After 20EFC:
 * sb 0 / lw / and 0xFFFFFCFF / sw D_8009D1AC; sw 0 D_8009D1A8;
 * sb 0 D_8009D1CE / D_8009D235; sw 0 D_8009D304; sh 0 D_8009D21C;
 * 10 pairs {0,-1} at 0x800A7FF0; 7 words at 0x800B8A90 (lui 0x800C
 * + signed addiu 0x8A90). Does not write mode 7 or +0xE.
 */
void func_80029810_remainder_cut(void)
{
    unsigned int word;
    unsigned int i;

    PE_StoreU8(0x8009D1ACu, 0u);
    word = PE_LoadU32(0x8009D1ACu);
    PE_StoreU32(0x8009D1A8u, 0u);
    PE_StoreU8(0x8009D1CEu, 0u);
    PE_StoreU8(0x8009D235u, 0u);
    PE_StoreU32(0x8009D304u, 0u);
    PE_StoreU16(0x8009D21Cu, 0u);
    PE_StoreU32(0x8009D1ACu, word & 0xFFFFFCFFu);
    for (i = 0; i < 10u; i++) {
        PE_StoreU16(0x800A7FF0u + i * 4u, 0u);
        PE_StoreU16(0x800A7FF2u + i * 4u, 0xFFFFu);
    }
    for (i = 0; i < 7u; i++)
        PE_StoreU32(0x800B8A90u + i * 4u, 0u);
}

/*
 * 29810 prologue 0x80029810..0x8002984B (15 words). lw D_8009D254,
 * lw *actor, sw 0 D_8009D1E8 / D_8009D290 / D_8009D28C (mode 0, not
 * 7), sb 0 D_8009CE7C / D_8009CE78 / D_8009D288 / D_8009CE74, sw
 * *actor → D_8009D278. Does not write 7 or +0xE.
 */
void func_80029810_prologue_cut(void)
{
    pe_addr_t actor;

    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(0x8009D1E8u, 0u);
    PE_StoreU32(0x8009D290u, 0u);
    PE_StoreU32(0x8009D28Cu, 0u);
    PE_StoreU8(0x8009CE7Cu, 0u);
    PE_StoreU8(0x8009CE78u, 0u);
    PE_StoreU8(0x8009D288u, 0u);
    PE_StoreU8(0x8009CE74u, 0u);
    PE_StoreU32(GA_RECORD_P, PE_LoadU32(actor));
}

/*
 * 29810 through 293F4: prologue + 20EFC + remainder + 71A64 +
 * 293F4(0) + after_hp tail. 144FC 0x3A is the sole jal.
 */
void func_80029810_cut(unsigned int encounter)
{
    func_80029810_prologue_cut();
    func_80020EFC();
    func_80029810_remainder_cut();
    func_80071A64(PE_LoadU32(0x8009D250u));
    func_800293F4_hp_cut();
    func_80029810_after_hp_cut(encounter);
}

/*
 * 144FC state 0x3A at 0x800145F8. D1A0 |= 2, then
 * lw v0, 0(s1); lbu a0, 0(v0); jal 29810. s1 is 144FC a0 =
 * binder arg0 (mode 0 = address of the immediate word).
 * NYPD 0x55(2) therefore does lbu(2), not lbu(overlay) and not
 * the encounter value 2. APPROXIMATION: host RAM[2]==0.
 * sb F4=0x3B, j park v0=0.
 */
int func_800144FC_state3A_cut(pe_addr_t arg0)
{
    pe_addr_t value;

    D_8009D1A0 |= 2u;
    value = PE_LoadU32(arg0);
    func_80029810_cut(PE_LoadU8(pe_kseg0(value)));
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0x3Bu);
    return 0;
}

/*
 * PE-BTL91 — opcode 0x55 144FC park-rewind wrapper.
 * Park epilogue 14660: v0=0, CE00-=0xC, delay=1.
 * Complete 3B takes 14658 v0=1. Type-6 +0xFC8 imm 2.
 * PE-BTL95 — jtbl[1..0x36] is also 14658; state>=0x3C parks.
 */
int func_800144FC(pe_addr_t args)
{
    uint8_t state;
    int done;
    pe_addr_t task;

    state = PE_LoadU8(GA_OVERLAY + 0xF4u);
    if (state == 0u)
        done = func_800144FC_state0_cut();
    else if (state == 0x37u)
        done = func_800144FC_state37_cut();
    else if (state == 0x38u)
        done = func_800144FC_state38_cut();
    else if (state == 0x39u)
        done = func_800144FC_state39_cut();
    else if (state == 0x3Au)
        done = func_800144FC_state3A_cut(args);
    else if (state == 0x3Bu)
        done = func_800144FC_state3B_cut();
    else if (state < 0x3Cu)
        done = 1; /* 14658: jtbl[1..0x36] */
    else
        done = 0; /* 14660: sltiu 0x3C fail */
    if (done == 0) {
        PE_StoreU32(0x8009CE00u, PE_LoadU32(0x8009CE00u) - 0xCu);
        task = PE_LoadU32(0x8009D300u);
        if (task != 0u)
            PE_StoreU32(task + 0x10u, 1u);
    }
    return done;
}

#define GA_GP_5C  0x8009CDCCu
#define GA_B0E64  0x800B0E64u
#define GA_D270   0x8009D270u

/*
 * func_80087198 is 5 words (0x80087198..0x800871AC): D_8009D270=1,
 * return 0. Twin of 87414. Matching src/ exists; this is the native
 * port. 6CDA4 state 0 a0==0 jals it after the table fill.
 */
int func_80087198(void)
{
    PE_StoreU32(GA_D270, 1u);
    return 0;
}

/*
 * func_80087414 is 5 words (0x80087414..0x80087428): D_8009D270=2,
 * return 0. Matching src/ exists; this is the native port. 6CDA4
 * state 0 a0==3 jals it after the table fill.
 */
int func_80087414(void)
{
    PE_StoreU32(GA_D270, 2u);
    return 0;
}

/*
 * func_8006D078 is 117 words (0x8006D078..0x8006D24C), not the
 * 357-word span to 6D60C (that includes 6D24C and 6D2B8).
 * +0xF3 JT 0x80011458: 0 / 0x28 / 0x29 / 0x2A / 0x2B; default v0=0.
 * No +0xE store. State 0: sw 0 → gp+0x5C, sb 0x28, re-dispatch.
 * State 0x28 jals 6CDA4(1,1,0,lw +0x194,0x21,0); v0==1 returns 1.
 * v0!=1 && +0x10>=2 sb 0x29 return 1; else sb 0x2A and re-dispatch.
 * 0x2A walks D_800B0E64 records (8B, count = word24>>16). Empty
 * or exhausted → sb 0 return 0. bit0x10 && lhu+4>=2 → sb 0x2B.
 * 0x2B jals 6CDA4(3,lhu+4,lhu+6,…). 87198 is not stubbed.
 */
void func_8006D078_state0_cut(void)
{
    PE_StoreU32(GA_GP_5C, 0u);
    PE_StoreU8(GA_OVERLAY + 0xF3u, 0x28u);
}

static pe_addr_t func_8006D078_rec(unsigned int index, unsigned int *count_out)
{
    pe_addr_t base;
    pe_addr_t s2;
    unsigned int word24;

    base = PE_LoadU32(GA_B0E64);
    if (base == 0) {
        if (count_out)
            *count_out = 0;
        return 0;
    }
    s2 = base + PE_LoadU32(base + 4u);
    word24 = PE_LoadU32(s2 + 0x24u);
    if (count_out)
        *count_out = word24 >> 16;
    return base + (word24 & 0x3FFFFFu) + index * 8u;
}

int func_8006D078_state2A_cut(void)
{
    unsigned int index;
    unsigned int count;
    pe_addr_t rec;

    index = PE_LoadU32(GA_GP_5C);
    rec = func_8006D078_rec(index, &count);
    if (rec == 0 || index >= count) {
        PE_StoreU8(GA_OVERLAY + 0xF3u, 0u);
        return 0;
    }
    if ((PE_LoadU8(rec + 3u) & 0x10u) != 0 && PE_LoadU16(rec + 4u) >= 2u) {
        PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Bu);
        return 1;
    }
    PE_StoreU32(GA_GP_5C, index + 1u);
    return 1;
}

int func_8006D078_state2B_cut(void)
{
    unsigned int index;
    pe_addr_t rec;
    pe_addr_t dest;

    index = PE_LoadU32(GA_GP_5C);
    rec = func_8006D078_rec(index, 0);
    dest = PE_LoadU32(GA_OVERLAY + 0x194u);
    if (func_8006CDA4(3, (int)PE_LoadU16(rec + 4u),
                      (int)PE_LoadU16(rec + 6u), dest, 0x21, 0) == 1)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Au);
    PE_StoreU32(GA_GP_5C, index + 1u);
    return 1;
}

int func_8006D078(void)
{
    unsigned int f3;
    unsigned int hops;

    f3 = PE_LoadU8(GA_OVERLAY + 0xF3u);
    if (f3 >= 44u)
        return 0;
    for (hops = 0; hops < 64u; hops++) {
        if (f3 == 0u) {
            func_8006D078_state0_cut();
            f3 = 0x28u;
        }
        if (f3 == 0x28u) {
            if (func_8006CDA4(1, 1, 0, PE_LoadU32(GA_OVERLAY + 0x194u),
                              0x21, 0) == 1)
                return 1;
            if (PE_LoadU8(GA_OVERLAY + 0x10u) >= 2u) {
                PE_StoreU8(GA_OVERLAY + 0xF3u, 0x29u);
                return 1;
            }
            PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Au);
            f3 = 0x2Au;
            continue;
        }
        if (f3 == 0x29u) {
            if (func_8006CDA4(1, (int)PE_LoadU8(GA_OVERLAY + 0x10u), 0,
                              PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
                return 1;
            PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Au);
            f3 = 0x2Au;
            continue;
        }
        if (f3 == 0x2Au) {
            if (func_8006D078_state2A_cut() == 0)
                return 0;
            f3 = PE_LoadU8(GA_OVERLAY + 0xF3u);
            if (f3 == 0x2Au)
                continue;
            continue;
        }
        if (f3 == 0x2Bu)
            return func_8006D078_state2B_cut();
        return 0;
    }
    return 1;
}

#define GA_XA_TABLE  0x8009317Cu
#define GA_PEIMG_LBA 0x800B0DD8u
#define GA_GP_400    0x8009D170u
#define GA_GP_404    0x8009D174u
#define GA_GP_408    0x8009D178u
#define GA_GP_40C    0x8009D17Cu

extern int func_8006E6D4(int lba_base, int lba_off, pe_addr_t dest, int size);
extern int func_8006E7E8(void);

/*
 * func_8006CDA4 is 181 words (0x8006CDA4..0x8006D078). +0xF0 JT
 * 0x80011428: 0 / 7 / 8 / 9 / 0xA; 1-6 unused. No +0xE store.
 * Live 6D078 0x28 is a0=1 a1=1: state 0 fills gp+0x400/404/408 from
 * D_8009317C + D_800B0DD8, skips 87198, sb 7, returns 1. a0==0
 * (6D60C F2=0x2F, a1=lb +0xE1=0x0D) calls 87198 (D_8009D270=1)
 * after the same table fill. a0==3 calls 87414 (D_8009D270=2).
 * Dest is lw overlay+0x194. State 7 jals real 6E6D4 with host
 * byte size (chunk<<11; retail a3 is sectors, proven by state 9
 * sll 11). -1 sb 0 return 0; else sb 8 return 1. State 8 jals
 * real 6E7E8: -1 sb 7, pending stays 8, 0 sb 9. State 9 live a0=1
 * jals real 87090(dest, 0); -1 sb 0, else sb 0xA and parks.
 * State 0xA jals real 870E0 (return D_8009D24C). -1 sb 0;
 * busy stays 0xA; 0 subtracts gp+0x40C from remain and sb 7.
 * 870E0 does not store; DMA-complete is not invented here.
 */
void func_8006CDA4_state0_a0eq1_cut(int a1)
{
    unsigned int idx;
    unsigned int half0;
    unsigned int half6;
    unsigned int lba;

    idx = ((unsigned int)a1) << 1;
    half0 = PE_LoadU16(GA_XA_TABLE + 4u + idx);
    half6 = PE_LoadU16(GA_XA_TABLE + idx + 6u);
    lba = PE_LoadU32(GA_PEIMG_LBA) + PE_LoadU32(GA_XA_TABLE) + half0;
    PE_StoreU32(GA_GP_404, half6 - half0);
    PE_StoreU32(GA_GP_408, half6 - half0);
    PE_StoreU32(GA_GP_400, lba);
    PE_StoreU8(GA_OVERLAY + 0xF0u, 7u);
}

int func_8006CDA4_state7_cut(pe_addr_t dest, int stack_len)
{
    unsigned int remain;
    unsigned int chunk;
    int issued;

    remain = PE_LoadU32(GA_GP_408);
    if (remain == 0u) {
        PE_StoreU8(GA_OVERLAY + 0xF0u, 0u);
        return 0;
    }
    chunk = remain;
    if ((unsigned int)stack_len < remain)
        chunk = (unsigned int)stack_len;
    PE_StoreU32(GA_GP_40C, chunk);
    issued = func_8006E6D4((int)PE_LoadU32(GA_GP_400),
                           (int)(PE_LoadU32(GA_GP_404) - remain), dest,
                           (int)(chunk << 11));
    if (issued == -1) {
        PE_StoreU8(GA_OVERLAY + 0xF0u, 0u);
        return 0;
    }
    PE_StoreU8(GA_OVERLAY + 0xF0u, 8u);
    return 1;
}

int func_8006CDA4_state8_cut(void)
{
    int st;

    st = func_8006E7E8();
    if (st == -1) {
        PE_StoreU8(GA_OVERLAY + 0xF0u, 7u);
        return 1;
    }
    if (st != 0)
        return 1;
    PE_StoreU8(GA_OVERLAY + 0xF0u, 9u);
    return 1;
}

int func_8006CDA4_state9_a0eq1_cut(pe_addr_t dest)
{
    int uploaded;

    uploaded = func_80087090(dest, 0);
    if (uploaded == -1) {
        PE_StoreU8(GA_OVERLAY + 0xF0u, 0u);
        return 1;
    }
    PE_StoreU8(GA_OVERLAY + 0xF0u, 0xAu);
    return 1;
}

int func_8006CDA4_stateA_cut(void)
{
    int st;
    unsigned int remain;
    unsigned int chunk;

    st = func_800870E0();
    if (st == -1) {
        PE_StoreU8(GA_OVERLAY + 0xF0u, 0u);
        return 1;
    }
    if (st != 0)
        return 1;
    remain = PE_LoadU32(GA_GP_408);
    chunk = PE_LoadU32(GA_GP_40C);
    PE_StoreU32(GA_GP_408, remain - chunk);
    PE_StoreU8(GA_OVERLAY + 0xF0u, 7u);
    return 1;
}

int func_8006CDA4(int a0, int a1, int a2, pe_addr_t a3, int stack_len,
                  int stack_flag)
{
    unsigned int f0;

    (void)a2;
    (void)stack_flag;
    f0 = PE_LoadU8(GA_OVERLAY + 0xF0u);
    if (f0 >= 11u)
        return 1;
    if (f0 == 0u) {
        func_8006CDA4_state0_a0eq1_cut(a1);
        if (a0 == 0)
            (void)func_80087198();
        else if (a0 == 3)
            (void)func_80087414();
        return 1;
    }
    if (f0 == 7u)
        return func_8006CDA4_state7_cut(a3, stack_len);
    if (f0 == 8u)
        return func_8006CDA4_state8_cut();
    if (f0 == 9u) {
        if (a0 == 1)
            return func_8006CDA4_state9_a0eq1_cut(a3);
        return 1;
    }
    if (f0 == 10u)
        return func_8006CDA4_stateA_cut();
    return 1;
}
