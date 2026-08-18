/*
 * PE-BTL100 — 2A7F8 mode-3: 2AA98 / 2B29C case 0.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 2A7F8 is a compare chain, not a jump table:
 *   mode 1 → 25EE8 (not this cut)
 *   mode 2 → 2B0E8 (victory phases → mode 9)
 *   mode 3 → record+0x4C bit 0x800 ? 2AA98
 *            : 53E6C(18) ? 2AA98 (+ 5409C if 2AA98!=0)
 *            : 2B29C
 *
 * 2AA98: 8-way jtbl 0x800108F0 on D_8009CE74 (gp+0x104).
 * 2B29C: 6-way jtbl 0x80010910 on the same byte.
 * Both case 0 wait for Aya +0x0E==19 and +0x0F==+0x16
 * (death clip from 1F078 / 1A680(19)). They DIVERGE:
 *   2AA98 complete: +0x98|=0x100, CE70=16, CE74++
 *   2B29C complete: walk D20C (not Aya), then 293F4(0),
 *                   CE70=70, CE74++, Aya+0x98|=0x100
 *   2B29C wait: Aya+0x98 &= ~0x100
 *
 * 2AA98 is the bit-0x800 / category-18 presentation.
 * 2B29C is the zero-fixture remaining-actor cleanup.
 * Cases 1-4 drain CE70 / +0x252. Case 5: 295E4 tail,
 * mode=-1, 6A25C dest 0xA9400048. Overlay / 21D4C /
 * 51510 / sound jals stay deferred.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D280 0x8009D280u
#define GA_D_8009D28C 0x8009D28Cu
#define GA_D_8009D1A0 0x8009D1A0u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D2A0 0x8009D2A0u /* gp+0x530 */
#define GA_D_8009D2A4 0x8009D2A4u /* gp+0x534 */
#define GA_D_8009D2EC 0x8009D2ECu /* gp+0x57C */
#define GA_D_8009CE70 0x8009CE70u /* gp+0x100 timer */
#define GA_D_8009CE74 0x8009CE74u /* gp+0x104 phase */
#define GA_D_800A77F4 0x800A77F4u
#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800B0CE6 0x800B0CE6u
#define GA_D_800B0D8A 0x800B0D8Au
#define GA_D_800915E0 0x800915E0u
#define DEST_GAMEOVER 0xA9400048u

static int pe_aya_death_clip_ready(pe_addr_t aya)
{
    if (aya == 0u)
        return 0;
    if (PE_LoadU8(aya + 0x0Eu) != 19u)
        func_8001A680_command_cut(aya, 19u);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya == 0u)
        return 0;
    return PE_LoadU8(aya + 0x0Fu) == (uint8_t)PE_LoadU16(aya + 0x16u);
}

int func_8002AA98(void)
{
    pe_addr_t aya;
    uint8_t phase;
    uint32_t flags;

    phase = PE_LoadU8(GA_D_8009CE74);
    if (phase >= 8u)
        return 0;
    if (phase != 0u)
        return 0;

    aya = PE_LoadU32(GA_D_8009D254);
    if (!pe_aya_death_clip_ready(aya)) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            flags = PE_LoadU32(aya + 0x98u);
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
        }
        return 0;
    }
    aya = PE_LoadU32(GA_D_8009D254);
    PE_StoreU8(GA_D_8009CE70, 16u);
    flags = PE_LoadU32(aya + 0x98u);
    PE_StoreU32(aya + 0x98u, flags | 0x100u);
    PE_StoreU8(GA_D_8009CE74, (uint8_t)(phase + 1u));
    return 0;
}

static void func_8002B29C_case0(void)
{
    pe_addr_t aya;
    pe_addr_t actor;
    uint32_t flags;

    aya = PE_LoadU32(GA_D_8009D254);
    if (!pe_aya_death_clip_ready(aya)) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            flags = PE_LoadU32(aya + 0x98u);
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
        }
        return;
    }

    actor = PE_LoadU32(GA_D_8009D20C);
    aya = PE_LoadU32(GA_D_8009D254);
    while (actor != 0u) {
        pe_addr_t body;
        pe_addr_t next;
        int8_t kind;

        next = PE_LoadU32(actor + 4u);
        if (actor != aya) {
            body = PE_LoadU32(actor);
            kind = 0;
            if (body != 0u)
                kind = (int8_t)PE_LoadU8(body + 5u);
            if (kind != 1) {
                if (body != 0u ||
                    (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u) {
                    uint16_t half;

                    half = PE_LoadU16(actor + 0x250u);
                    PE_StoreU16(actor + 0x250u, (uint16_t)(half | 2u));
                    flags = PE_LoadU32(actor + 0x98u) | 0x1000u;
                    PE_StoreU32(actor + 0x98u, flags);
                    if (body != 0u) {
                        unsigned int cmd;

                        cmd = (unsigned int)(uint16_t)(int16_t)
                              (int8_t)PE_LoadU8(body + 6u);
                        func_8001A680_command_cut(actor, cmd);
                        flags = PE_LoadU32(actor + 0x98u);
                        if ((flags & 0x40000000u) != 0u &&
                            PE_LoadU8(body + 0xAFu) == 0u) {
                            PE_StoreU32(actor + 0x98u, flags | 0x10u);
                            PE_StoreU32(actor, 0u);
                        }
                    }
                    PE_StoreU32(actor + 0x68u, 0u);
                    PE_StoreU32(actor + 0x6Cu, 0u);
                    PE_StoreU32(actor + 0x70u, 0u);
                }
            }
        }
        actor = next;
    }

    func_800293F4_hp_cut();
    aya = PE_LoadU32(GA_D_8009D254);
    PE_StoreU8(GA_D_8009CE70, 70u);
    PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
    if (aya != 0u) {
        flags = PE_LoadU32(aya + 0x98u);
        PE_StoreU32(aya + 0x98u, flags | 0x100u);
    }
}

/* 2B29C[1] CE70!=60 walk: remaining actors get +0x98 bit 0x10. */
static void func_8002B29C_flag10_walk(void)
{
    pe_addr_t aya;
    pe_addr_t actor;

    aya = PE_LoadU32(GA_D_8009D254);
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        pe_addr_t next;
        pe_addr_t body;

        next = PE_LoadU32(actor + 4u);
        if (actor != aya) {
            body = PE_LoadU32(actor);
            if (body != 0u ||
                (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u) {
                if (PE_LoadU8(actor + 0x252u) == 0u) {
                    uint32_t flags = PE_LoadU32(actor + 0x98u);
                    PE_StoreU32(actor + 0x98u, flags | 0x10u);
                }
            }
        }
        actor = next;
    }
}

/*
 * 295E4 tail (21D4C / 51510 / HUD sb storm deferred).
 * sb 0 gp+0x57C / +0x530, Aya+0x194 = D_800915E0,
 * D1A0 &= ~2, D2E8 &= ~0x10, B0CE6 |= 2.
 */
static void func_800295E4_tail_cut(void)
{
    pe_addr_t aya;
    uint32_t v;

    PE_StoreU8(GA_D_8009D2EC, 0u);
    PE_StoreU8(GA_D_8009D2A0, 0u);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u)
        PE_StoreU32(aya + 0x194u, PE_LoadU32(GA_D_800915E0));
    D_8009D1A0 &= ~2u;
    PE_StoreU32(GA_D_8009D1A0, PE_LoadU32(GA_D_8009D1A0) & ~2u);
    v = PE_LoadU32(GA_D_8009D2E8);
    PE_StoreU32(GA_D_8009D2E8, v & ~0x10u);
    PE_StoreU8(GA_D_800B0CE6, (uint8_t)(PE_LoadU8(GA_D_800B0CE6) | 2u));
}

/*
 * 6A25C dest restore. Sound/CD jals (81268 / 86F34 / 86FF8 /
 * 87024 / 85744 / 38D0C / 39970) stay fail-closed.
 */
void func_8006A25C(void)
{
    uint32_t old;

    /* D280 is host-owned for 1220C/3F3C4; guest word is the
     * PE_Load view. Retail has one location — write both. */
    old = D_8009D280;
    if (old == 0u)
        old = PE_LoadU32(GA_D_8009D280);
    PE_StoreU32(GA_D_800A77F4, old);
    D_8009D280 = DEST_GAMEOVER;
    PE_StoreU32(GA_D_8009D280, DEST_GAMEOVER);
    PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) | 0x100u);
}

static void func_8002B29C_case1(void)
{
    uint8_t timer;

    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer == 60u) {
        pe_addr_t aya = PE_LoadU32(GA_D_8009D254);
        pe_addr_t actor = PE_LoadU32(GA_D_8009D20C);

        while (actor != 0u) {
            pe_addr_t next = PE_LoadU32(actor + 4u);
            pe_addr_t body = PE_LoadU32(actor);

            if (actor != aya) {
                int8_t kind = 0;
                if (body != 0u)
                    kind = (int8_t)PE_LoadU8(body + 5u);
                if (kind != 1 &&
                    (body != 0u ||
                     (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u))
                    func_8003C5D8(actor + 0x1B4u, 60);
            }
            actor = next;
        }
    } else {
        func_8002B29C_flag10_walk();
    }
    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer != 0u) {
        PE_StoreU8(GA_D_8009CE70, (uint8_t)(timer - 1u));
        return;
    }
    PE_StoreU8(GA_D_8009CE70, 30u);
    PE_StoreU8(GA_D_8009CE74,
               (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
}

static void func_8002B29C_timer_advance(uint8_t next_timer)
{
    uint8_t timer;

    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer != 0u) {
        PE_StoreU8(GA_D_8009CE70, (uint8_t)(timer - 1u));
        return;
    }
    PE_StoreU8(GA_D_8009CE70, next_timer);
    PE_StoreU8(GA_D_8009CE74,
               (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
}

void func_8002B29C(void)
{
    uint8_t phase;
    pe_addr_t aya;

    phase = PE_LoadU8(GA_D_8009CE74);
    if (phase >= 6u)
        return;
    if (phase == 0u) {
        func_8002B29C_case0();
        return;
    }
    if (phase == 1u) {
        func_8002B29C_case1();
        return;
    }
    if (phase == 2u) {
        func_8002B29C_timer_advance(80u);
        return;
    }
    if (phase == 3u) {
        func_8002B29C_timer_advance(60u);
        return;
    }
    if (phase == 4u) {
        aya = PE_LoadU32(GA_D_8009D254);
        if ((aya == 0u || PE_LoadU8(aya + 0x252u) == 0u) &&
            PE_LoadU8(GA_D_800B0D8A) == 0u) {
            PE_StoreU8(GA_D_8009CE74, 5u);
            return;
        }
        {
            uint8_t timer = PE_LoadU8(GA_D_8009CE70);
            if (timer != 0u)
                PE_StoreU8(GA_D_8009CE70, (uint8_t)(timer - 1u));
        }
        return;
    }
    /* phase 5: 2B8E8 jal 295E4; sw -1 mode; jal 6A25C */
    func_800295E4_tail_cut();
    PE_StoreU32(GA_D_8009D28C, 0xFFFFFFFFu);
    func_8006A25C();
}

/*
 * 2B0E8 — mode 2 (encounter-end / victory), not player death.
 * Phase on D_8009CE74:
 *   0: wait Aya+0x16==10 or D1A0&0x800; +0x98|=0x100;
 *      703F4 / 67CBC deferred; jal 4B70C; phase++
 *   1: wait gp+0x534==1000; phase++; +0x98&=~0x100
 *   2: wait +0x0F==+0x1A; 1A680(0x15) or 0x18 if D1A0&0x1800;
 *      clear 0x1800; phase++
 *   3: 6D60C(0)==1 wait; else 295E4, mode=9, B0CD8&=~0x8000
 */
/*
 * 2F300 named cut: jal 293F4(0) then sw mode 2 @ 2F570.
 * HUD sb storm / 1A680 / 6DE80 / 5218C stay deferred.
 */
void func_8002F300_mode2_cut(void)
{
    func_800293F4_hp_cut();
    PE_StoreU32(GA_D_8009D28C, 2u);
}

/*
 * 292EC remaining-enemy tail (inside 28E94).
 * D2A0!=0 → out. Walk D20C: any non-Aya with body keeps
 * combat. If none remain and record+0x0C>0 → 2F300.
 * Aya HP<=0 does not start victory (player death is mode 3).
 */
void func_800292EC_victory_ready_cut(void)
{
    pe_addr_t actor;
    pe_addr_t aya;
    pe_addr_t rec;
    int remain;

    if ((int8_t)PE_LoadU8(GA_D_8009D2A0) != 0)
        return;
    actor = PE_LoadU32(GA_D_8009D20C);
    remain = 1;
    aya = PE_LoadU32(GA_D_8009D254);
    while (actor != 0u) {
        if (actor != aya && PE_LoadU32(actor) != 0u)
            remain = 0;
        actor = PE_LoadU32(actor + 4u);
    }
    if (remain == 0)
        return;
    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    if ((int16_t)PE_LoadU16(rec + 0x0Cu) <= 0)
        return;
    func_8002F300_mode2_cut();
}

void func_8002B0E8(void)
{
    uint8_t phase;
    pe_addr_t aya;
    uint32_t flags;
    uint32_t d1a0;
    unsigned int cmd;

    phase = PE_LoadU8(GA_D_8009CE74);
    aya = PE_LoadU32(GA_D_8009D254);
    d1a0 = D_8009D1A0 | PE_LoadU32(GA_D_8009D1A0);

    if (phase == 0u) {
        if (aya == 0u)
            return;
        if (PE_LoadU16(aya + 0x16u) != 10u && (d1a0 & 0x800u) == 0u)
            return;
        flags = PE_LoadU32(aya + 0x98u);
        PE_StoreU32(aya + 0x98u, flags | 0x100u);
        /* 703F4 / 67CBC stay deferred. 4B70C installs 4BB80. */
        func_8004B70C(PE_LoadU32(0x8009D304u),
                      PE_LoadU16(0x8009D21Cu),
                      0x800A7FF0u);
        PE_StoreU8(GA_D_8009CE74, 1u);
        return;
    }
    if (phase == 1u) {
        if ((int16_t)PE_LoadU16(GA_D_8009D2A4) != 1000)
            return;
        PE_StoreU8(GA_D_8009CE74, 2u);
        if (aya != 0u) {
            flags = PE_LoadU32(aya + 0x98u);
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
        }
        return;
    }
    if (phase == 2u) {
        if (aya == 0u)
            return;
        if (PE_LoadU8(aya + 0x0Fu) != (uint8_t)PE_LoadU16(aya + 0x1Au))
            return;
        if ((d1a0 & 0x1800u) != 0u) {
            d1a0 &= ~0x1800u;
            D_8009D1A0 = d1a0;
            PE_StoreU32(GA_D_8009D1A0, d1a0);
            cmd = 0x18u;
        } else {
            cmd = 0x15u;
        }
        func_8001A680_command_cut(aya, cmd);
        PE_StoreU8(GA_D_8009CE74, 3u);
        return;
    }
    if (phase != 3u)
        return;
    if (func_8006D60C(0) == 1)
        return;
    func_800295E4_tail_cut();
    PE_StoreU32(GA_D_8009D28C, 9u);
    flags = PE_LoadU32(GA_D_800B0CD8) & ~0x8000u;
    PE_StoreU32(GA_D_800B0CD8, flags);
}

void func_8002A7F8_mode3_cut(void)
{
    pe_addr_t rec;
    uint32_t flags;
    int occ;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    flags = PE_LoadU32(rec + 0x4Cu);
    if (flags & 0x800u) {
        if (func_8002AA98() != 0)
            PE_StoreU32(rec + 0x4Cu, flags & ~0x800u);
        return;
    }
    occ = func_80053E6C(18);
    if (occ != 0) {
        (void)func_8002AA98();
        return;
    }
    func_8002B29C();
}
