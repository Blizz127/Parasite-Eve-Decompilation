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
 * when D2E8 bit 0 is clear (0x3F). PE-BTL68 indexes
 * table[*codep] (not actor+0) and jalrs 710A4/7136C.
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
 * jal 361F4, then integrate +0x68/+0x78/+0x58 into pose.
 * +0x98 bit 1 only adds +0x88 first; it does not skip pose.
 *
 * func_800361F4 — 24 words 0x800361F4..0x80036254. sw actor →
 * gp+0x580 (D_8009D2F0), then three words at +0xA0 into
 * D_8009D300. Nonempty slots jal 17018. Live 35038 zeros
 * +0xA0/+0xA4 and stores the 12700 task at +0xA8.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D26C 0x8009D26Cu
#define GA_D_8009D1F4 0x8009D1F4u
#define GA_D_8009D1E4 0x8009D1E4u
#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800943C0 0x800943C0u
#define GA_D_800BD000 0x800BD000u
#define GA_D_800BD020 0x800BD020u
#define GA_D_800BE9A0 0x800BE9A0u
#define GA_VT_35E04   0x80035E04u
#define GA_VT_35C84   0x80035C84u
#define GA_FN_7136C   0x8007136Cu
#define GA_FN_710A4   0x800710A4u
#define GA_WALK_VEL   0x801221A0u
#define CMD_RTIR_SF0  0x041E012u
#define CMD_RTIR_SF1  0x049E012u

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
    /* ROM 35C84/35E04: bit 1 only adds +0x88/+0x8C/+0x90.
     * Pose += +0x68/+0x78/+0x58 always. BTL11 skip-all is REJECTED. */
    if ((PE_LoadU32(actor + 0x98u) & 2u) != 0u) {
        PE_StoreU32(actor + 0x68u,
                    PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x88u));
        PE_StoreU32(actor + 0x6Cu,
                    PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x8Cu));
        PE_StoreU32(actor + 0x70u,
                    PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x90u));
    }
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
 * PE-BTL68 — 78934 GTE large-vector RT*IR (88w 0x80078934..0x80078A94).
 * Loads 5-word MATRIX at a0, splits each a1 word into >>15 / &0x7FFF,
 * MVMVA sf=0 then sf=1, adds (hi_mac<<3)+lo_mac into a2[0..2].
 */
void func_80078934(pe_addr_t matrix, pe_addr_t src, pe_addr_t dst)
{
    int i;
    int32_t hi[3];
    int32_t lo[3];

    PE_GTE_LoadRT33(matrix);
    for (i = 0; i < 3; i++) {
        int32_t v = (int32_t)PE_LoadU32(src + (unsigned)i * 4u);
        int32_t a = v;

        if (a < 0)
            a = -a;
        hi[i] = a >> 15;
        lo[i] = a & 0x7FFF;
        if (v < 0) {
            hi[i] = -hi[i];
            lo[i] = -lo[i];
        }
    }
    PE_GTE_SetIR((int16_t)hi[0], (int16_t)hi[1], (int16_t)hi[2]);
    PE_GTE_MVMVA(CMD_RTIR_SF0);
    hi[0] = g_pe_gte.mac[0];
    hi[1] = g_pe_gte.mac[1];
    hi[2] = g_pe_gte.mac[2];
    PE_GTE_SetIR((int16_t)lo[0], (int16_t)lo[1], (int16_t)lo[2]);
    PE_GTE_MVMVA(CMD_RTIR_SF1);
    for (i = 0; i < 3; i++) {
        int32_t h = hi[i];

        if (h < 0)
            h = -((-h) << 3);
        else
            h <<= 3;
        PE_StoreU32(dst + (unsigned)i * 4u,
                    (uint32_t)(g_pe_gte.mac[i] + h));
    }
}

static void pe_walk_digital_apply(pe_addr_t actor, uint32_t speed)
{
    uint32_t bits;
    uint16_t yaw;
    uint16_t facing;
    uint32_t diag;
    uint32_t vx;
    uint32_t vy;
    uint32_t vz;
    pe_addr_t rec;

    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u)
        return;

    diag = func_8003708C(speed, 0xB504u);
    bits = PE_LoadU32(GA_D_8009D26C);
    yaw = PE_LoadU16(GA_D_800BD020);
    facing = yaw;
    vx = 0u;
    vy = 0u;
    vz = 0u;
    if ((bits & 8u) != 0u) {
        if ((bits & 0x40u) != 0u) {
            facing = (uint16_t)(yaw + 0x600u);
            vx = (uint32_t)-(int32_t)diag;
            vz = diag;
        } else if ((bits & 0x10u) != 0u) {
            facing = (uint16_t)(yaw + 0xA00u);
            vx = diag;
            vz = diag;
        } else {
            facing = (uint16_t)(yaw + 0x800u);
            vz = speed;
        }
    } else if ((bits & 0x20u) != 0u) {
        if ((bits & 0x40u) != 0u) {
            facing = (uint16_t)(yaw + 0x200u);
            vx = (uint32_t)-(int32_t)diag;
            vz = (uint32_t)-(int32_t)diag;
        } else if ((bits & 0x10u) != 0u) {
            facing = (uint16_t)(yaw + 0xE00u);
            vx = diag;
            vz = (uint32_t)-(int32_t)diag;
        } else {
            facing = yaw;
            vz = (uint32_t)-(int32_t)speed;
        }
    } else if ((bits & 0x40u) != 0u) {
        facing = (uint16_t)(yaw + 0x400u);
        vx = (uint32_t)-(int32_t)speed;
    } else if ((bits & 0x10u) != 0u) {
        facing = (uint16_t)(yaw + 0xC00u);
        vx = speed;
    }

    rec = PE_LoadU32(GA_D_8009D254);
    if (rec != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        rec = PE_LoadU32(rec);
        facing = (uint16_t)(facing + 0x800u);
        if (rec != 0u)
            facing = (uint16_t)(facing
                                + ((PE_LoadU32(rec + 0x4Cu) >> 7) & 0xC00u));
    }
    facing &= 0xFFFu;
    PE_StoreU32(GA_WALK_VEL, vx);
    PE_StoreU32(GA_WALK_VEL + 4u, vy);
    PE_StoreU32(GA_WALK_VEL + 8u, vz);
    PE_StoreU16(actor + 0x3Au, facing);
    func_80078934(GA_D_800BD000, GA_WALK_VEL, actor + 0x68u);
    if ((PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        PE_StoreU16(actor + 0x3Au,
                    (uint16_t)((0x1000u - PE_LoadU16(actor + 0x3Au))
                               & 0xFFFu));
        PE_StoreU32(actor + 0x68u,
                    (uint32_t)-(int32_t)PE_LoadU32(actor + 0x68u));
    }
}

/*
 * PE-BTL68 — func_8007136C walk core.
 * 206 words 0x8007136C..0x800716A4, SHA-256 below in oracle.
 * Analog (BE9A0&0xF000==0x7000) is not this cut.
 */
void func_8007136C(pe_addr_t actor, pe_addr_t codep)
{
    uint32_t speed;
    uint32_t scaled;
    pe_addr_t rec;
    uint32_t word;
    uint32_t code;

    speed = 0x168000u;
    if ((D_8009D1A0 & 2u) != 0u) {
        rec = PE_LoadU32(actor);
        word = PE_LoadU32(rec + 0x4Cu);
        if ((word & 0xC0u) == 0x40u)
            speed = 0xB4000u;
        else if ((word & 0x100u) != 0u)
            speed = 0x21C000u;
        code = PE_LoadU8(rec + 0x13u);
        if (PE_LoadU32(codep) != code) {
            func_8001A680_command_cut(actor, code);
            PE_StoreU32(codep, code);
        }
    } else {
        code = PE_LoadU32(codep);
        if (code != 0x17u) {
            func_8001A680_command_cut(actor, 0x17u);
            PE_StoreU32(codep, 0x17u);
        }
    }
    scaled = func_8003708C(speed, PE_LoadU32(actor + 0x20u));
    scaled = func_8003708C(scaled,
                           (uint32_t)PE_LoadU16(actor + 0x26u) << 4);
    pe_walk_digital_apply(actor, scaled);
}

/*
 * PE-BTL68 — func_800710A4 d-pad walk (no Circle).
 * 178 words 0x800710A4..0x8007136C. D1A0&2 jals 7136C.
 */
void func_800710A4(pe_addr_t actor, pe_addr_t codep)
{
    uint32_t scaled;

    if ((D_8009D1A0 & 2u) != 0u) {
        func_8007136C(actor, codep);
        return;
    }
    if (PE_LoadU32(codep) != 0x16u) {
        func_8001A680_command_cut(actor, 0x16u);
        PE_StoreU32(codep, 0x16u);
    }
    scaled = func_8003708C(0x50000u, PE_LoadU32(actor + 0x20u));
    scaled = func_8003708C(scaled,
                           (uint32_t)PE_LoadU16(actor + 0x26u) << 4);
    pe_walk_digital_apply(actor, scaled);
}

static void pe_3999c_jalr(uint32_t fn, pe_addr_t actor, pe_addr_t codep)
{
    if (fn == GA_FN_7136C)
        func_8007136C(actor, codep);
    else if (fn == GA_FN_710A4)
        func_800710A4(actor, codep);
}

/*
 * PE-BTL67/68 — func_8003999C pad-table dispatcher.
 *
 * 118 words 0x8003999C..0x80039B74, SHA-256 9a5267b0…3f6f.
 * Sole TEXT caller 35C84 @ 35D14. Indexes table[*a2], not actor+0
 * (399C0 is lw 0(s2); s2=a2). BTL67 host-guard on actor+0 is
 * REJECTED. Record+0xC jalr of 710A4/7136C is this cut;
 * 71034/716A4/71754 are not.
 */
void func_8003999C(pe_addr_t actor, pe_addr_t table, pe_addr_t codep)
{
    uint32_t idx;
    pe_addr_t list;
    pe_addr_t rec;
    uint32_t flags;
    uint32_t fn;
    uint32_t mask1;
    uint32_t mask2;
    uint32_t matched;
    int take;

    if (actor == 0u || table == 0u || codep == 0u)
        return;
    idx = PE_LoadU32(codep);
    if (idx >= 64u)
        return;
    list = PE_LoadU32(table + idx * 4u);
    rec = list;
    flags = (list != 0u) ? PE_LoadU32(list) : 0u;
    if (flags == 0u)
        goto end_default;

    matched = 0u;
    while (flags != 0u) {
        fn = PE_LoadU32(rec + 0xCu);
        if (fn == 0u)
            return;
        mask1 = PE_LoadU32(rec + 4u);
        mask2 = PE_LoadU32(rec + 8u);
        take = 0;
        if ((flags & 0x20u) != 0u) {
            take = 1;
        } else if ((flags & 1u) != 0u) {
            if ((PE_LoadU32(GA_D_8009D26C) & mask1) == mask1) {
                if ((flags & 4u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
                else if ((flags & 8u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
                else if ((flags & 2u) == 0u)
                    take = 1;
                else
                    take = (PE_LoadU32(GA_D_8009D26C) & mask2) != 0u;
            }
        } else if ((flags & 2u) != 0u) {
            if ((PE_LoadU32(GA_D_8009D26C) & mask2) != 0u) {
                if ((flags & 4u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
                else if ((flags & 8u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
                else
                    take = 1;
            }
        } else if ((flags & 4u) != 0u) {
            take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
        } else if ((flags & 8u) != 0u) {
            take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
        }
        if (take) {
            pe_3999c_jalr(fn, actor, codep);
            matched = 1u;
        }
        if ((flags & 0x10u) == 0u && matched != 0u)
            return;
        rec += 16u;
        flags = PE_LoadU32(rec);
    }

end_default:
    if (rec == 0u)
        return;
    fn = PE_LoadU32(rec + 0xCu);
    if (fn != 0u)
        pe_3999c_jalr(fn, actor, codep);
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
