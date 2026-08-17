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

void func_80029810_after_hp_cut(unsigned int encounter)
{
    pe_addr_t record;
    pe_addr_t actor;
    pe_addr_t source;
    int value;
    int cap;

    Bootstrap_ReturnVoid("func_800209F0", "func_80029810_after_hp_cut");
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

    Bootstrap_ReturnVoid("func_80030640", "func_80029810_after_hp_cut");
    actor = PE_LoadU32(GA_ACTOR_P);
    PE_StoreU32(actor + 0x194u, CALLBACK_2D268);
    source = PE_LoadU32(actor + 0x238u);
    PE_StoreU16(GA_VALUE_D27C,
                (uint16_t)(PE_LoadU32(source + 0x18u) - 100u));
    Bootstrap_ReturnVoid1("func_800339A0", "func_80029810_after_hp_cut",
                          encounter & 0xFFu);
    record = PE_LoadU32(GA_RECORD_P);
    func_8001A680_command_cut(actor, PE_LoadU8(record + 0x12u));
}

int func_800144FC_state3B_cut(void)
{
    pe_addr_t actor;
    unsigned int flags;

    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) != 0u)
        return 0;
    actor = PE_LoadU32(GA_OVERLAY);
    PE_StoreU8(GA_OVERLAY + 0xF4u, 0u);
    flags = PE_LoadU32(actor) & 0xFF7FFFFFu;
    PE_StoreU32(actor, flags);
    return 1;
}

/*
 * 144FC state 0 at 0x80014544. After 0x3B stores +0xF4=0 the next
 * dispatch lands here. If (+0xE & 3)==0, sb 0x37 → +0xF4; always
 * ori 0x00800000 on *actor ($s1 = a0; native uses overlay word 0
 * like state 3B). j 0x80014660 returns 0 and rewinds the script PC.
 * Does not complete 0x55.
 */
int func_800144FC_state0_cut(void)
{
    pe_addr_t actor;

    if ((PE_LoadU8(GA_OVERLAY + 0x0Eu) & 3u) == 0u)
        PE_StoreU8(GA_OVERLAY + 0xF4u, 0x37u);
    actor = PE_LoadU32(GA_OVERLAY);
    PE_StoreU32(actor, PE_LoadU32(actor) | 0x00800000u);
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

#define GA_GP_5C  0x8009CDCCu

/*
 * func_8006D078 is 117 words (0x8006D078..0x8006D24C), not the
 * 357-word span to 6D60C (that includes 6D24C and 6D2B8).
 * +0xF3 JT 0x80011458: 0 / 0x28 / 0x29 / 0x2A / 0x2B; default v0=0.
 * No +0xE store. State 0: sw 0 → gp+0x5C, sb 0x28, re-dispatch.
 * State 0x28 jals 6CDA4(1,1,0,lw +0x194,0x21,0); v0==1 returns 1.
 * v0!=1 && +0x10<2 sb 0x2A. 0x2A walk / 6E7E8 are not stubbed.
 */
void func_8006D078_state0_cut(void)
{
    PE_StoreU32(GA_GP_5C, 0u);
    PE_StoreU8(GA_OVERLAY + 0xF3u, 0x28u);
}

int func_8006D078(void)
{
    unsigned int f3;

    f3 = PE_LoadU8(GA_OVERLAY + 0xF3u);
    if (f3 >= 44u)
        return 0;
    if (f3 == 0u) {
        func_8006D078_state0_cut();
        f3 = 0x28u;
    }
    if (f3 == 0x28u) {
        if (func_8006CDA4(1, 1, 0, PE_LoadU32(GA_OVERLAY + 0x194u), 0x21, 0) == 1)
            return 1;
        if (PE_LoadU8(GA_OVERLAY + 0x10u) >= 2u) {
            PE_StoreU8(GA_OVERLAY + 0xF3u, 0x29u);
            return 1;
        }
        PE_StoreU8(GA_OVERLAY + 0xF3u, 0x2Au);
        return 1;
    }
    if (f3 == 0x29u || f3 == 0x2Au || f3 == 0x2Bu)
        return 1;
    return 0;
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
 * D_8009317C + D_800B0DD8, skips 87198/87414, sb 7, returns 1.
 * Dest is lw overlay+0x194. State 7 jals real 6E6D4 with host
 * byte size (chunk<<11; retail a3 is sectors, proven by state 9
 * sll 11). -1 sb 0 return 0; else sb 8 return 1. State 8 jals
 * real 6E7E8: -1 sb 7, pending stays 8, 0 sb 9 and parks.
 * 87090/state 9 are not stubbed.
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
        if (a0 == 0 || a0 == 3)
            return 1;
        func_8006CDA4_state0_a0eq1_cut(a1);
        return 1;
    }
    if (f0 == 7u)
        return func_8006CDA4_state7_cut(a3, stack_len);
    if (f0 == 8u)
        return func_8006CDA4_state8_cut();
    if (f0 == 9u || f0 == 10u)
        return 1;
    return 1;
}
