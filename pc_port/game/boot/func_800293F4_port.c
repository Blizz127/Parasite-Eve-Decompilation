/*
 * PE-BTL2 — func_800293F4_hp_cut: Aya-record HP copy at battle init
 *
 * Named cut of func_800293F4 (full body 124 words / 0x1F0, exclusive
 * jr at 0x800295DC). This translation is ONLY the HP-layout prefix
 * reached from opcode 0x55 → func_800144FC state 0x3A → jal
 * func_80029810 → jal func_800293F4(0):
 *
 *   0x800293F4..0x80029448 exclusive (21 words / 0x54, file 0x19BF4).
 *   First excluded word: addiu $v0, 1 at 0x80029448 (a0==1 branch;
 *   29810 passes a0=0, so that arm is not this cut).
 *
 * Words dumped from SHA-1-exact build/disc1.candidate.exe
 * (452fb033f2eaa4b18aa20a5bca60b8125af3a37b). yaml [0x11718, asm].
 * No matching src/ (no era/asm split in this worktree).
 *
 * ROM (record = *(gp+0x508) = D_8009D278):
 *
 *   lh  +0x1C
 *   lh  +0x0C
 *   if (+0x1C) < (+0x0C): sh +0x1C into +0x0C     signed clamp
 *   sb  4 at +0x12
 *   lhu +0x0C
 *   sw  $zero, 0x460($gp)                          D_8009D1D0
 *   sh  $zero, +0x10
 *   sw  $zero, +0x34
 *   sh  +0x0C into +0x0E                           copy
 *
 * Independent reader at 0x80029350: lh D_8009D278+0x0C ; blez
 * (alive/HP gate on the same +0x0C halfword). Default rodata
 * D_80010928 has +0x0C/+0x0E/+0x1C all 0x002D. Opcode 0x5A tag 4
 * sh +0x0C and tag 5 sh +0x0E (func_8002FF78); 0x59 selector 4
 * reads +0x0C, selector 5 is a nop (func_8002FE78 JT).
 *
 * Do not invent ATB, menus, damage, PE, AI, 0x55 completion, or a
 * first-command word. sb 4 at +0x12 is in this window (1A680 clip
 * argument); it is not a battle-menu command. 29810 passes a0=0, so
 * the a0==1 arm's sb 1 at gp+0x4D4 (0x80029464) is not this cut;
 * 299CC's active body stays idle.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D278 0x8009D278u /* gp+0x508 current record */
#define GA_D_8009D1D0 0x8009D1D0u /* gp+0x460 */

void func_800293F4_hp_cut(void)
{
    pe_addr_t rec;
    int16_t cap;
    int16_t hp;
    uint16_t cur;

    rec = PE_LoadU32(GA_D_8009D278);
    cap = (int16_t)PE_LoadU16(rec + 0x1Cu);
    hp = (int16_t)PE_LoadU16(rec + 0x0Cu);
    if (cap < hp) {
        PE_StoreU16(rec + 0x0Cu, (uint16_t)cap);
    }
    rec = PE_LoadU32(GA_D_8009D278);
    PE_StoreU8(rec + 0x12u, 4u);
    rec = PE_LoadU32(GA_D_8009D278);
    cur = PE_LoadU16(rec + 0x0Cu);
    PE_StoreU32(GA_D_8009D1D0, 0u);
    PE_StoreU16(rec + 0x10u, 0u);
    PE_StoreU32(rec + 0x34u, 0u);
    PE_StoreU16(rec + 0x0Eu, cur);
}

/* Complete command cancellation / status reset used by battle results.
 * The initialization-only prefix above retains its historical callers. */
void func_80021D4C(void)
{
    unsigned index=PE_LoadU8(0x8009D1D4u);
    extern int func_80053D2C(int item);
    while ((index&255u)<PE_LoadU8(0x8009CE3Cu)) {
        uint32_t action=PE_LoadU16(0x800BE834u+(index&255u)*8u);
        if (action-3u<384u) (void)func_80053D2C((int16_t)action-3);
        index++;
    }
    PE_StoreU8(0x8009CE3Cu,0u); PE_StoreU8(0x8009D1D4u,0u);
    D_8009D1A0&=~0x100u;
    PE_StoreU32(0x8009D1A0u,PE_LoadU32(0x8009D1A0u)&~0x100u);
}

void func_800293F4(uint32_t mode)
{
    pe_addr_t rec;
    uint32_t flags;
    func_800293F4_hp_cut();
    rec=PE_LoadU32(GA_D_8009D278); flags=PE_LoadU32(rec+0x4Cu);
    if ((mode&255u)==1u) {
        PE_StoreU8(0x8009D234u,90u); PE_StoreU8(0x8009D244u,1u); flags|=0x800000u;
    } else { PE_StoreU8(0x8009D244u,0u); flags&=~0xC00000u; }
    PE_StoreU32(rec+0x4Cu,flags&0xC0C00000u);
    PE_StoreU8(rec+0x56u,0u); PE_StoreU8(rec+0x5Eu,0u); PE_StoreU8(rec+0x66u,0u);
    PE_StoreU32(0x8009D2E8u,PE_LoadU32(0x8009D2E8u)&~0x10u);
    func_80021D4C(); func_800374E8();
    PE_StoreU16(0x8009D298u,0u); PE_StoreU8(0x8009D29Au,0u);
    PE_StoreU8(0x8009D29Bu,0u); PE_StoreU32(0x8009D29Cu,0u);
}
