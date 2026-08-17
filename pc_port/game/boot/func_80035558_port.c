/*
 * PE-BTL11 — func_80035558 D20C walk cut, type!=0 vtable 35E04,
 * and 361F4 slot publish (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80035558 — 459 words 0x80035558..0x80035C84, SHA-256
 * 7352fc04…ba83. Sole TEXT caller 3F3C4 @ 0x8003F4F0 (field
 * tick; 3F3C4 sole caller 0x800123D8). This cut is the prologue
 * only:
 *
 *   if (D_8009D1A0 & 4) return;   # ROM skips walk and continues
 *   for actor in D_8009D20C via +4:
 *       jalr actor+0x190 (a0=actor)
 *
 * Known jalr targets 0x80035E04 (types 1–9) and 0x80035C84
 * (type 0) are ported below. PE-BTL67 wires 35C84 jal 3999C
 * when D2E8 bit 0 is clear (0x3F). 7136C-family jalr is not
 * this cut.
 * After the walk, this cut also takes the live D1A0&2 jal
 * 299CC @ 0x800355E8 (consume + after-consume idle gate) and
 * the 35B2C jal 69594 event pump. PE-BTL66 adds the post-69594
 * 1A4AC clip ticks @ 35B84 (D254 when D1A0&0x100) and 35BEC
 * (D20C walk when that bit is clear; skip +0x98 & 0x800040).
 * 6C5BC @ 35B24, 661CC @ 35B34, and the 355B4–35B20 mid-body
 * are not this cut. Not M2.
 *
 * func_80035E04 — 83 words 0x80035E04..0x80035F50. If D1A0 bit
 * 0x100, only 361F4. Else snapshot +0x28/+0x38 into +0x40/+0x50,
 * jal 361F4, then if +0x98 bit 1 integrate motion. Live 35038
 * empty-+0x1AC leaves +0x98 bit 1 clear.
 *
 * func_800361F4 — 24 words 0x800361F4..0x80036254. sw actor →
 * gp+0x580 (D_8009D2F0), then three words at +0xA0 into
 * D_8009D300. Nonempty slots jal 17018. Live 35038 zeros
 * +0xA0/+0xA4 and stores the 12700 task at +0xA8.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800943C0 0x800943C0u
#define GA_VT_35E04   0x80035E04u
#define GA_VT_35C84   0x80035C84u

extern void func_8001A4AC(pe_addr_t actor);

static void pe_actor_snapshot_pose(pe_addr_t actor)
{
    PE_StoreU32(actor + 0x40u, PE_LoadU32(actor + 0x28u));
    PE_StoreU32(actor + 0x44u, PE_LoadU32(actor + 0x2Cu));
    PE_StoreU32(actor + 0x48u, PE_LoadU32(actor + 0x30u));
    PE_StoreU16(actor + 0x50u, PE_LoadU16(actor + 0x38u));
    PE_StoreU16(actor + 0x52u, PE_LoadU16(actor + 0x3Au));
    PE_StoreU16(actor + 0x54u, PE_LoadU16(actor + 0x3Cu));
}

static void pe_actor_integrate_motion(pe_addr_t actor)
{
    if ((PE_LoadU32(actor + 0x98u) & 2u) == 0u)
        return;

    PE_StoreU32(actor + 0x68u,
                PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x88u));
    PE_StoreU32(actor + 0x6Cu,
                PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x8Cu));
    PE_StoreU32(actor + 0x70u,
                PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x90u));
    PE_StoreU32(actor + 0x68u,
                PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x78u));
    PE_StoreU32(actor + 0x6Cu,
                PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x7Cu));
    PE_StoreU32(actor + 0x70u,
                PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x80u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x68u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x6Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x70u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x58u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x5Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x60u));
}

extern unsigned int D_8009D1A0;

void func_800361F4(pe_addr_t actor)
{
    unsigned int i;
    uint32_t slot;

    PE_StoreU32(GA_D_8009D2F0, actor);
    for (i = 0; i < 3u; i++) {
        slot = PE_LoadU32(actor + 0xA0u + i * 4u);
        PE_StoreU32(GA_D_8009D300, slot);
        if (slot != 0u)
            func_80017018();
    }
}

void func_80035E04(pe_addr_t actor)
{
    if (D_8009D1A0 & 0x100u) {
        func_800361F4(actor);
        return;
    }

    pe_actor_snapshot_pose(actor);
    func_800361F4(actor);
    pe_actor_integrate_motion(actor);
}

/*
 * PE-BTL67 — func_8003999C pad-table dispatcher.
 *
 * 118 words 0x8003999C..0x80039B74, SHA-256 9a5267b0…3f6f.
 * Sole TEXT caller 35C84 @ 35D14. Zero jal; jalr of record+0xC
 * (7136C / 710A4 / 71754 / 716A4) is not this cut.
 *
 * a0=actor, a1=D_800943C0, a2=&code (actor+0x0E or 0x11).
 * ROM indexes table[actor+0]. 2F76C stores 0x800B8A20 there;
 * table only has rows 4/5/16/17/21/22/23. Host returns when
 * the index is not a 0..63 row (live pointer would fault).
 * Do not substitute *a2 for actor+0.
 */
void func_8003999C(pe_addr_t actor, pe_addr_t table, pe_addr_t codep)
{
    uint32_t idx;
    pe_addr_t list;

    (void)codep;
    if (actor == 0u || table == 0u)
        return;
    idx = PE_LoadU32(actor);
    if (idx >= 64u)
        return;
    list = PE_LoadU32(table + idx * 4u);
    if (list == 0u)
        return;
    /* Mask walk + jalr record+0xC: not this cut. */
}

void func_80035C84(pe_addr_t actor)
{
    uint32_t code;
    pe_addr_t rec;

    pe_actor_snapshot_pose(actor);
    func_800361F4(actor);
    if ((PE_LoadU32(GA_D_8009D2E8) & 1u) == 0u) {
        code = PE_LoadU8(actor + 0x0Eu);
        rec = PE_LoadU32(GA_D_8009D254);
        if (rec != 0u) {
            rec = PE_LoadU32(rec);
            if (rec != 0u && (PE_LoadU32(rec + 0x4Cu) & 0xC0u) == 0x80u)
                code = 0x11u;
        }
        PE_StoreU32(0x80122190u, code);
        func_8003999C(actor, GA_D_800943C0, 0x80122190u);
    }
    pe_actor_integrate_motion(actor);
}

void func_80035558_walk_cut(void)
{
    pe_addr_t actor;
    pe_addr_t fn;

    if (D_8009D1A0 & 4u)
        return;

    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        fn = PE_LoadU32(actor + 0x190u);
        if (fn == GA_VT_35E04)
            func_80035E04(actor);
        else if (fn == GA_VT_35C84)
            func_80035C84(actor);
        actor = PE_LoadU32(actor + 4u);
    }

    if (D_8009D1A0 & 2u) {
        func_800299CC_consume_cut();
        func_800299CC_after_consume_cut();
    }
    func_80069594();

    /* ROM 0x80035B44: D1A0&4 skips both 1A4AC sites. */
    if (D_8009D1A0 & 4u)
        return;
    if (D_8009D1A0 & 0x100u) {
        if ((PE_LoadU32(GA_D_800B0CD8) & 0x40000u) == 0u) {
            actor = PE_LoadU32(GA_D_8009D254);
            if (actor != 0u)
                func_8001A4AC(actor);
        }
        return;
    }
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        if (!(actor == PE_LoadU32(GA_D_8009D254)
              && (PE_LoadU32(GA_D_800B0CD8) & 0x40000u) != 0u)
            && (PE_LoadU32(actor + 0x98u) & 0x800040u) == 0u)
            func_8001A4AC(actor);
        actor = PE_LoadU32(actor + 4u);
    }
}
