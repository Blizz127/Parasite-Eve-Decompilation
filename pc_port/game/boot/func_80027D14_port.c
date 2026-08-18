/*
 * PE-BTL105 — 27D14 enemy tick and 28E94 death phases.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 27D14: 534 words 0x80027D14..0x80028570. a0 = actor, *actor = body.
 * After 1D340, 299CC @ 0x8002A53C walks non-Aya bodies here.
 * body&0x6000==0x2000 jals 28574 (Attack HP subtract).
 * body+0x10<=0 and !(D1A0&0x100) and kind not 1/3 → zero
 * +0x68/6C/70, jal 28E94(actor).
 *
 * 28E94: 317 words. Phase on body+0xAC. Phase 0 decrements D2A0
 * and falls into phase 1. body+0xAF==0 skips to phase 3.
 * Phase 3: +0x98|=0x410, 2F970(actor) nulls the body, then
 * 292EC remaining-enemy tail. 866A4 / 3C5D8 / 3CAEC / 27A08
 * stay deferred.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D1A0 0x8009D1A0u
#define GA_D_8009D2A0 0x8009D2A0u
#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D294 0x8009D294u
#define GA_D_8009D1D4 0x8009D1D4u
#define GA_D_8009CE54 0x8009CE54u
#define GA_D_8009CE55 0x8009CE55u
#define GA_D_8009CE48 0x8009CE48u
#define GA_D_8009CE4C 0x8009CE4Cu
#define GA_D_8009CE3C 0x8009CE3Cu
#define GA_D_8009CE38 0x8009CE38u
#define GA_D_8009CE39 0x8009CE39u
#define GA_D_8009D25C 0x8009D25Cu
#define GA_T_800BE830 0x800BE830u
#define GA_D_800A5D58 0x800A5D58u
#define SLOT_STRIDE   220u
#define BODY_HIT      0x2000u
#define BODY_REACT    0x4000u
#define BODY_HITMASK  0x6000u

extern unsigned int D_8009D1A0;

void func_8002F970(pe_addr_t p)
{
    pe_addr_t want;
    unsigned int i;

    if (p == 0u)
        return;
    want = PE_LoadU32(p);
    for (i = 0u; i < 7u; i++) {
        pe_addr_t rec = GA_D_800A5D58 + i * SLOT_STRIDE;

        if (rec + 4u == want)
            PE_StoreU32(rec, 0u);
    }
    PE_StoreU32(p, 0u);
}

static uint32_t pe_d1a0(void)
{
    return D_8009D1A0 | PE_LoadU32(GA_D_8009D1A0);
}

void func_80028E94(pe_addr_t actor)
{
    pe_addr_t body;
    uint8_t phase;
    uint32_t flags;
    pe_addr_t walk;

    if (actor == 0u)
        return;
    body = PE_LoadU32(actor);
    if (body == 0u)
        return;
    phase = PE_LoadU8(body + 0xACu);
    if (phase == 0u) {
        int8_t busy;

        busy = (int8_t)PE_LoadU8(GA_D_8009D2A0);
        PE_StoreU8(GA_D_8009D2A0, (uint8_t)(busy - 1));
        phase = 1u;
        PE_StoreU8(body + 0xACu, 1u);
    }
    if (phase == 1u) {
        if (PE_LoadU8(body + 0xAFu) == 0u) {
            /* 866A4 sound/CD deferred. */
            PE_StoreU8(body + 0xACu, 3u);
            return;
        }
        /* AF!=0 clip/3C5D8 path is not this cut. */
        return;
    }
    if (phase == 2u)
        return;
    if (phase != 3u)
        return;

    flags = PE_LoadU32(actor + 0x98u) | 0x410u;
    PE_StoreU32(actor + 0x98u, flags);
    func_8002F970(actor);

    walk = PE_LoadU32(GA_D_8009D20C);
    while (walk != 0u) {
        pe_addr_t next = PE_LoadU32(walk + 4u);

        if (PE_LoadU32(walk) == 0u &&
            PE_LoadU32(walk + 0x18Cu) == actor) {
            flags = PE_LoadU32(walk + 0x98u);
            PE_StoreU32(walk + 0x98u, flags | 0x10u);
        }
        walk = next;
    }
    /* Persist EXP walk: zero +0xA0 skips to 292D8. */
    func_800292EC_victory_ready_cut();
}

/*
 * PE-BTL109 — 23008 sets D294 so 299CC @ 2A4D4 jals 236E8.
 * weapon+6==8 or ammo (weapon+0x0C & 0x3FF) → D294=1.
 */
void func_80023008(void)
{
    pe_addr_t rec;
    pe_addr_t weapon;
    int16_t kind;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    weapon = PE_LoadU32(rec + 0x68u);
    if (weapon == 0u)
        return;
    kind = (int16_t)PE_LoadU16(weapon + 6u);
    if (kind == 8) {
        PE_StoreU8(GA_D_8009D294, 1u);
        return;
    }
    if ((PE_LoadU32(weapon + 0x0Cu) & 0x3FFu) != 0u) {
        PE_StoreU8(GA_D_8009D294, 1u);
        return;
    }
    PE_StoreU8(GA_D_8009D294, 0u);
}

/*
 * PE-BTL110 — 2312C kinds 6/8/10: clip +0x0F==+0x16 and
 * gp+0xC8/C9==0 jals 23008. 21F38 always reaches 2312C
 * unless Aya+0x0E==12. 21DE0: CE3C!=0 && D1D4<CE3C &&
 * Aya+0x0E>=4 && slot+4<3 → 21F38.
 */
int func_8002312C(pe_addr_t slot)
{
    pe_addr_t aya;
    uint8_t kind;

    (void)slot;
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya == 0u)
        return 0;
    kind = PE_LoadU8(aya + 0x0Eu);
    if (kind < 6u || kind > 11u)
        return 0;
    if ((kind & 1u) != 0u)
        return 0;
    if ((uint32_t)PE_LoadU8(aya + 0x0Fu) !=
        (uint32_t)PE_LoadU16(aya + 0x16u))
        return 0;
    if (PE_LoadU8(GA_D_8009CE38) != 0u ||
        PE_LoadU8(GA_D_8009CE39) != 0u)
        return 0;
    func_80023008();
    return 1;
}

void func_80021F38(void)
{
    pe_addr_t aya;
    pe_addr_t slot;

    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u && PE_LoadU8(aya + 0x0Eu) == 12u)
        return;
    slot = GA_T_800BE830 + ((uint32_t)PE_LoadU8(GA_D_8009D1D4) << 3);
    (void)func_8002312C(slot);
}

/*
 * PE-BTL114 — 24A3C is the 17-way at 0x80010824. Entry is
 * 24A3C (lbu D25C), not 24A40. Sole TEXT jal is 22394 @
 * 229D8. Case 9 is the only CE54 writer: wait Aya
 * +0x0F==+0x1A, 1A680((int8)CE48*2+9), CE55=2, CE54=1,
 * body |= 0x2000. Cases 0-3 increment D25C (6C1CC and
 * HUD jals deferred). Cases 4-8 / 10-16 stay deferred.
 */
int func_80024A3C(void)
{
    pe_addr_t aya;
    pe_addr_t actor;
    pe_addr_t body;
    uint8_t phase;
    unsigned int cmd;
    uint32_t word;

    phase = PE_LoadU8(GA_D_8009D25C);
    if (phase >= 0x11u)
        return 0;
    if (phase == 0u) {
        /* Case 0: 6C1CC/3C5D8/702DC/6F39C deferred. sb D25C+1. */
        PE_StoreU8(GA_D_8009D25C, 1u);
        return 0;
    }
    if (phase == 1u) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u && PE_LoadU8(aya + 0x252u) != 0u)
            return 0;
        if (PE_LoadU8(0x800B0D8Au) != 0u)
            return 0;
        PE_StoreU8(GA_D_8009D25C, 2u);
        return 0;
    }
    if (phase == 2u) {
        uint32_t flags;

        /* 6C1CC/3C5D8/6F39C deferred (treat 6C1CC ready). */
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u)
            return 0;
        func_8001A680_command_cut(aya, 5u);
        PE_StoreU8(aya + 0x252u, 1u);
        flags = PE_LoadU32(aya + 0x98u) | 0x100u;
        PE_StoreU32(aya + 0x98u, flags);
        PE_StoreU16(aya + 0x250u, (uint16_t)(PE_LoadU16(aya + 0x250u) | 4u));
        PE_StoreU16(GA_D_8009CE4C, 30u);
        PE_StoreU8(GA_D_8009D25C, 3u);
        return 0;
    }
    if (phase == 3u) {
        int16_t timer;

        timer = (int16_t)PE_LoadU16(GA_D_8009CE4C);
        if (timer != 0) {
            PE_StoreU16(GA_D_8009CE4C, (uint16_t)(timer - 1));
            return 0;
        }
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            uint32_t flags = PE_LoadU32(aya + 0x98u) & ~0x100u;

            PE_StoreU32(aya + 0x98u, flags);
        }
        PE_StoreU8(GA_D_8009D25C, 4u);
        return 0;
    }
    if (phase != 9u)
        return 0;
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya == 0u)
        return 0;
    if (PE_LoadU8(aya + 0x0Fu) != (uint8_t)PE_LoadU16(aya + 0x1Au))
        return 0;
    cmd = ((unsigned int)(int)(int8_t)PE_LoadU8(GA_D_8009CE48) << 1) + 9u;
    func_8001A680_command_cut(aya, cmd & 0xFFFFu);
    PE_StoreU8(GA_D_8009CE55, 2u);
    PE_StoreU8(GA_D_8009CE54, 1u);
    actor = PE_LoadU32(GA_T_800BE830 +
                       ((uint32_t)PE_LoadU8(GA_D_8009D1D4) << 3));
    if (actor != 0u) {
        body = PE_LoadU32(actor);
        if (body != 0u) {
            word = PE_LoadU32(body);
            word = (word & ~BODY_HITMASK) | BODY_HIT;
            PE_StoreU32(body, word);
        }
    }
    PE_StoreU8(GA_D_8009CE48, (uint8_t)(PE_LoadU8(GA_D_8009CE48) + 1u));
    return 0;
}

/*
 * 22394: lb D2A0. Zero walks/clears targeting (not this cut).
 * Nonzero and rec+0x4C&0x80000 jals 24A3C. 0x200000 memcpy
 * prefix and 53D2C stay deferred.
 */
void func_80022394(void)
{
    pe_addr_t rec;

    if ((int8_t)PE_LoadU8(GA_D_8009D2A0) == 0)
        return;
    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    if ((PE_LoadU32(rec + 0x4Cu) & 0x80000u) == 0u)
        return;
    (void)func_80024A3C();
}

void func_80021DE0(void)
{
    uint8_t count;
    uint8_t idx;
    pe_addr_t aya;
    pe_addr_t slot;
    int16_t tid;

    count = PE_LoadU8(GA_D_8009CE3C);
    idx = PE_LoadU8(GA_D_8009D1D4);
    if (count == 0u || idx >= count) {
        PE_StoreU8(GA_D_8009D1D4, 0u);
        PE_StoreU8(GA_D_8009CE3C, 0u);
        return;
    }
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u) {
        PE_StoreU32(aya + 0x68u, 0u);
        PE_StoreU32(aya + 0x6Cu, 0u);
        PE_StoreU32(aya + 0x70u, 0u);
        if (PE_LoadU8(aya + 0x0Eu) < 4u)
            return;
    }
    slot = GA_T_800BE830 + ((uint32_t)idx << 3);
    tid = (int16_t)PE_LoadU16(slot + 4u);
    if (tid < 3)
        func_80021F38();
    else if (tid >= 387 && tid < 407)
        func_80022394();
}

/*
 * 236E8: 457 words. Live 0x2000 arm: table[D1D4].actor body
 * |= 0x2000 when gp+0xE4==1 and weapon+6 is 6 or 8.
 * 23DE8 clears D294. 21278 / 6DE80 stay deferred.
 */
void func_800236E8(void)
{
    pe_addr_t rec;
    pe_addr_t weapon;
    pe_addr_t actor;
    pe_addr_t body;
    int16_t kind;
    uint32_t word;
    unsigned int idx;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    weapon = PE_LoadU32(rec + 0x68u);
    if (weapon == 0u)
        return;
    kind = (int16_t)PE_LoadU16(weapon + 6u);
    if (kind != 6 && kind != 8)
        return;
    if ((int8_t)PE_LoadU8(GA_D_8009CE54) != 1)
        return;
    idx = PE_LoadU8(GA_D_8009D1D4);
    actor = PE_LoadU32(GA_T_800BE830 + (idx << 3));
    if (actor == 0u)
        return;
    body = PE_LoadU32(actor);
    if (body == 0u)
        return;
    word = PE_LoadU32(body);
    word = (word & ~BODY_HITMASK) | BODY_HIT;
    PE_StoreU32(body, word);
    PE_StoreU8(GA_D_8009D294, 0u);
}

static int32_t pe_s32_div100(int32_t n)
{
    int64_t prod = (int64_t)n * (int64_t)(int32_t)0x51EB851F;
    int32_t hi = (int32_t)(prod >> 32);

    return (hi >> 5) - (n >> 31);
}

/*
 * 28574: 437 words. Default path (rec+0x4C bits 8 and 0x10 clear):
 * a1 = ((rec+0x1E)/5 + weapon+0) * ROM[0x800108E4][weapon+0x10&0xf] / 100
 * s0 = a1 - body+0x8C (weapon+6==8 or flags 0x180000).
 * s0<=0 → chip 1 or 2. Always sw body+0x10 -= s0, then
 * body = (body & ~0x6000) | 0x4000. FP bit-8 path fail-closed.
 */
void func_80028574(pe_addr_t actor)
{
    pe_addr_t body;
    pe_addr_t rec;
    pe_addr_t weapon;
    uint32_t flags;
    uint32_t word;
    unsigned int idx;
    uint8_t scale;
    int32_t atk;
    int32_t dmg;
    int32_t hp;

    if (actor == 0u)
        return;
    body = PE_LoadU32(actor);
    if (body == 0u)
        return;
    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    flags = PE_LoadU32(rec + 0x4Cu);
    if ((flags & 8u) != 0u || (flags & 0x10u) != 0u)
        return;
    weapon = PE_LoadU32(rec + 0x68u);
    if (weapon == 0u)
        return;

    {
        static const uint8_t rom_scale[11] = {
            0, 100, 60, 41, 0, 25, 0, 18, 0, 0, 13
        };
        uint32_t atb = PE_LoadU16(rec + 0x1Eu);
        uint32_t fifth = (uint32_t)(((uint64_t)atb * 0xCCCCCCCDull) >> 34);

        idx = PE_LoadU32(weapon + 0x10u) & 0xFu;
        scale = (idx < 11u) ? rom_scale[idx] : 0u;
        atk = (int32_t)fifth + (int32_t)(int16_t)PE_LoadU16(weapon);
        atk = pe_s32_div100(atk * (int32_t)scale);
    }

    if ((flags & 0x180000u) != 0u ||
        (int16_t)PE_LoadU16(weapon + 6u) == 8)
        dmg = atk - (int32_t)PE_LoadU16(body + 0x8Cu);
    else {
        unsigned int div = PE_LoadU32(weapon + 0x10u) & 0xFu;

        if (div == 0u)
            dmg = atk;
        else
            dmg = atk - (int32_t)PE_LoadU16(body + 0x8Cu) / (int)div;
    }

    if (dmg <= 0) {
        word = PE_LoadU32(body);
        dmg = ((word & 0x38000u) == 0x18000u) ? 2 : 1;
        if ((word & 0xC0000u) == 0xC0000u)
            PE_StoreU32(body + 0xCCu,
                        PE_LoadU32(body + 0xCCu) | 0x1000000u);
    }

    hp = (int32_t)PE_LoadU32(body + 0x10u) - dmg;
    PE_StoreU32(body + 0x10u, (uint32_t)hp);
    word = PE_LoadU32(body);
    word = (word & ~BODY_HITMASK) | BODY_REACT;
    PE_StoreU32(body, word);
}

static void pe_27d14_atb(pe_addr_t actor, pe_addr_t body)
{
    uint16_t cur;
    uint16_t rate;

    /* s1 stays *actor. ATB is body+0x0C / +0x8E. */
    cur = PE_LoadU16(body + 0x0Cu);
    rate = PE_LoadU16(body + 0x8Eu);
    if (cur < 0x2328u) {
        uint16_t sum = (uint16_t)(cur + rate);

        if ((PE_LoadU32(body) & 1u) != 0u)
            sum = (uint16_t)((int)sum - ((int)rate * 8) / 5);
        PE_StoreU16(body + 0x0Cu, sum);
        return;
    }
    PE_StoreU16(body + 0x0Cu, 0x2328u);
    if ((PE_LoadU32(actor + 0x98u) & 0x1000u) != 0u)
        PE_StoreU16(body + 0x0Cu, 0u);
}

static void pe_27d14_dot(pe_addr_t body)
{
    uint32_t word;
    unsigned int shift;
    int32_t hp;
    int16_t tick;

    word = PE_LoadU32(body);
    if ((word & 0x10u) == 0u)
        return;
    shift = (word >> 5) & 0x1Fu;
    if (shift < 0x1Eu) {
        shift = (shift + 1u) & 0x1Fu;
        word = (word & 0xFFFFFC1Fu) | (shift << 5);
        PE_StoreU32(body, word);
        return;
    }
    tick = (int16_t)PE_LoadU16(body + 0x96u);
    hp = (int32_t)PE_LoadU32(body + 0x10u) - (int32_t)tick;
    PE_StoreU32(body + 0x10u, (uint32_t)hp);
    PE_StoreU32(body, word & 0xFFFFFC1Fu);
}

void func_80027D14(pe_addr_t actor)
{
    pe_addr_t body;
    pe_addr_t body_keep;
    int32_t hp;
    int32_t cap;
    int8_t kind;
    uint32_t d1a0;
    uint32_t bits;

    if (actor == 0u)
        return;
    body = PE_LoadU32(actor);
    if (body == 0u)
        return;
    body_keep = body;

    hp = (int32_t)PE_LoadU32(body + 0x10u);
    cap = (int32_t)PE_LoadU32(body + 0x88u);
    if (cap < hp) {
        hp = cap;
        PE_StoreU32(body + 0x10u, (uint32_t)hp);
    }

    d1a0 = pe_d1a0();
    if ((d1a0 & 0x100u) == 0u) {
        pe_27d14_atb(actor, body);
        pe_27d14_dot(body);
    }

    /* body&0x6000==0x2000 → 28574. 0x4000 → 28088 clears 0x6000.
     * 27A08 / 6DCE4 / 1A680 react stay deferred. */
    bits = PE_LoadU32(body) & BODY_HITMASK;
    if (bits == BODY_HIT)
        func_80028574(actor);
    else if (bits == BODY_REACT)
        PE_StoreU32(body, PE_LoadU32(body) & ~BODY_HITMASK);
    kind = (int8_t)PE_LoadU8(body + 5u);
    if (kind != 1) {
        PE_StoreU32(actor + 0x68u, 0u);
        PE_StoreU32(actor + 0x6Cu, 0u);
        PE_StoreU32(actor + 0x70u, 0u);
    }
    if ((PE_LoadU32(body) & 0x400u) != 0u)
        PE_StoreU32(body + 0x10u, 0xFFFFFFFFu);

    hp = (int32_t)PE_LoadU32(body + 0x10u);
    if (hp > 0) {
        PE_StoreU32(body + 0x14u, (uint32_t)hp);
        return;
    }
    if ((d1a0 & 0x100u) != 0u) {
        PE_StoreU32(body + 0x14u, (uint32_t)hp);
        return;
    }

    kind = (int8_t)PE_LoadU8(body + 5u);
    if (kind != 1 && kind != 3) {
        PE_StoreU32(actor + 0x68u, 0u);
        PE_StoreU32(actor + 0x6Cu, 0u);
        PE_StoreU32(actor + 0x70u, 0u);
        func_80028E94(actor);
    }
    /* 2854C uses the entry $s1 body even after 2F970 nulls *actor. */
    PE_StoreU32(body_keep + 0x14u, PE_LoadU32(body_keep + 0x10u));
}
