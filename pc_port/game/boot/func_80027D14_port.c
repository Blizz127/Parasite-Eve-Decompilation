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
 * 28E94 is complete: death clip/fade, linked actors, rewards, drops
 * and the remaining-enemy gate into 2F300's victory initialization.
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
#define GA_D_8009D258 0x8009D258u
#define GA_OVERLAY    0x800B0CD8u
#define GA_T_800BE830 0x800BE830u
#define GA_D_800A5D58 0x800A5D58u
#define SLOT_STRIDE   220u
#define BODY_HIT      0x2000u
#define BODY_REACT    0x4000u
#define BODY_HITMASK  0x6000u



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
    pe_addr_t body=PE_LoadU32(actor),walk;
    uint8_t phase=PE_LoadU8(body+0xACu);
    unsigned i;
    if (phase == 0u) {
        PE_StoreU8(GA_D_8009D2A0,(uint8_t)(PE_LoadU8(GA_D_8009D2A0)-1u));
        phase=1u; PE_StoreU8(body+0xACu,1u);
    }
    if (phase == 1u) {
        if (!PE_LoadU8(body+0xAFu)) {
            func_800866A4(0u,PE_LoadU32(body+8u));
            PE_StoreU8(body+0xACu,3u); return;
        }
        if (PE_LoadU16(actor+0x16u)<PE_LoadU8(body+0x91u) &&
            !PE_LoadU8(actor+14u) && (int8_t)PE_LoadU8(body+5u)<2) return;
        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x100u);
        func_8003C5D8(actor+0x1B4u,22);
        PE_StoreU8(body+0xACu,2u); PE_StoreU8(body+0xADu,0u);
        PE_StoreU32(actor+0x68u,0u); PE_StoreU32(actor+0x6Cu,0u); PE_StoreU32(actor+0x70u,0u);
        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
        func_800866A4(0u,PE_LoadU32(body+8u));
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u))
            if (!PE_LoadU32(walk) && PE_LoadU32(walk+0x18Cu)==actor)
                func_8003C5D8(walk+0x1B4u,22);
        return;
    }
    if (phase == 2u) {
        uint8_t color=(uint8_t)(-128-(int)PE_LoadU8(body+0xADu)*16);
        func_8003CAEC(actor+0x1B4u,color,128u,color);
        if (!color) {
            PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|2u);
            PE_StoreU8(body+0xACu,3u); PE_StoreU8(body+0xADu,0u);
        } else {
            if (!PE_LoadU8(body+0xADu) && PE_LoadU8(body+0xAEu))
                func_8006DED4(PE_LoadU32(0x800B0E64u),PE_LoadU16(body+0xB4u),0,
                    (int16_t)PE_LoadU16(actor+0x268u),(int16_t)PE_LoadU16(actor+0x26Au),
                    (int16_t)PE_LoadU16(actor+0x26Cu));
            PE_StoreU8(body+0xADu,(uint8_t)(PE_LoadU8(body+0xADu)+1u));
        }
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
            if (PE_LoadU32(walk) || PE_LoadU32(walk+0x18Cu)!=actor) continue;
            func_8003CAEC(walk+0x1B4u,color,128u,color);
            if (!color) PE_StoreU16(walk+0x250u,PE_LoadU16(walk+0x250u)|2u);
        }
        return;
    }
    if (phase!=3u || (PE_LoadU8(actor+0x252u) && PE_LoadU8(body+0xAFu))) return;
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x410u);
    func_8002F970(actor);
    for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u))
        if (!PE_LoadU32(walk) && PE_LoadU32(walk+0x18Cu)==actor)
            PE_StoreU32(walk+0x98u,PE_LoadU32(walk+0x98u)|0x10u);
    PE_StoreU32(0x8009D304u,PE_LoadU32(0x8009D304u)+PE_LoadU16(body+14u));
    {
        int32_t reward=(int32_t)((uint32_t)PE_LoadU16(body+0x98u)-
            (uint32_t)PE_LoadU16(body+0x9Au)*PE_LoadU16(body+0x9Cu));
        if (reward>0) PE_StoreU16(0x8009D21Cu,(uint16_t)(PE_LoadU16(0x8009D21Cu)+(uint32_t)reward));
    }
    for (i=0;i<10;i++) if (!(int16_t)PE_LoadU16(0x800A7FF0u+i*4u)) {
        PE_StoreU16(0x800A7FF0u+i*4u,PE_LoadU8(body+0x9Eu)); break;
    }
    if ((int16_t)PE_LoadU16(body+0xA0u))
        for (i=0;i<10;i++) if (!(int16_t)PE_LoadU16(0x800A7FF0u+i*4u)) {
            PE_StoreU16(0x800A7FF0u+i*4u,PE_LoadU16(body+0xA0u));
            if ((int16_t)PE_LoadU16(body+0xA2u)>=0)
                PE_StoreU16(0x800A7FF2u+i*4u,PE_LoadU16(body+0xA2u));
            break;
        }
    func_800292EC_victory_ready_cut();
}

/*
 * PE-BTL114 — 24A3C is the 17-way at 0x80010824. Entry is
 * 24A3C (lbu D25C), not 24A40. Sole TEXT jal is 22394 @
 * 229D8. Case 9 is the only CE54 writer: wait Aya
 * +0x0F==+0x1A, 1A680((int8)CE48*2+9), CE55=2, CE54=1,
 * body |= 0x2000. Cases 0-8 increment D25C. 6C1CC / 6FC18 /
 * 6F39C / 6DE80 stay deferred. Case 6 waits Aya+0x252==0.
 * That byte is dest+0x9E on the embedded Aya+0x1B4 dest.
 * 35558 jals 3AF14 → 3C818, which sb $0 at dest+0x9E when
 * +0x8C==1. Do not plant the clear.
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
        uint32_t flags;
        int ev;

        (void)func_8006C1CC(1);
        PE_StoreU8(GA_D_8009CE48, 0u);
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            unsigned i;
            for (i = 0; i < 3; i++) {
                PE_StoreU32(0x8009E054u + i * 4u, PE_LoadU32(aya + 40u + i * 4u));
                PE_StoreU16(0x8009CE58u + i * 2u, PE_LoadU16(aya + 56u + i * 2u));
            }
            PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) & ~4u);
            flags = PE_LoadU32(aya + 0x98u) | 0x80u;
            PE_StoreU32(aya + 0x98u, flags);
            func_8003C5D8(aya + 0x1B4u, 30);
            PE_StoreU16(aya + 0x250u,
                        (uint16_t)(PE_LoadU16(aya + 0x250u) | 2u));
            func_8003C5D8(GA_OVERLAY + 0x14u, 30);
            PE_StoreU16(0x800B0D88u, (uint16_t)(PE_LoadU16(0x800B0D88u) | 2u));
            (void)func_800702DC();
            if (PE_LoadU32(0x800942E0u) != 0u) {
                ev = func_8006F39C(0x6Bu, aya);
                PE_StoreU32(GA_D_8009D258, (uint32_t)ev);
            }
        }
        PE_StoreU8(GA_D_8009D25C, 1u);
        return 0;
    }
    if (phase == 1u) {
        (void)func_8006C1CC(1);
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

        if (func_8006C1CC(1) != 0)
            return 0;
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u)
            return 0;
        func_8001A680_command_cut(aya, 5u);
        PE_StoreU8(aya + 0x252u, 1u);
        flags = PE_LoadU32(aya + 0x98u) | 0x100u;
        PE_StoreU32(aya + 0x98u, flags);
        func_8003C5D8(aya + 0x1B4u, 30);
        PE_StoreU16(aya + 0x250u, (uint16_t)(PE_LoadU16(aya + 0x250u) | 4u));
        if (PE_LoadU32(0x800942E0u) != 0u)
            (void)func_8006F39C(0x6Cu, aya);
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
    if (phase == 4u) {
        pe_addr_t walk;

        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u)
            return 0;
        if (PE_LoadU8(aya + 0x0Fu) != (uint8_t)PE_LoadU16(aya + 0x16u))
            return 0;
        walk = PE_LoadU32(GA_D_8009D20C);
        while (walk != 0u) {
            pe_addr_t next = PE_LoadU32(walk + 4u);

            if (walk != aya && PE_LoadU32(walk) != 0u) {
                PE_StoreU32(walk + 0x68u, 0u);
                PE_StoreU32(walk + 0x6Cu, 0u);
                PE_StoreU32(walk + 0x70u, 0u);
            }
            walk = next;
        }
        /* 6FC18 / 6F39C(109) deferred. */
        PE_StoreU8(GA_D_8009D25C, 5u);
        return 0;
    }
    if (phase == 5u) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u)
            return 0;
        func_8001A680_command_cut(aya, 6u);
        func_8003C5D8(aya + 0x1B4u, 15);
        PE_StoreU16(aya + 0x250u, (uint16_t)(PE_LoadU16(aya + 0x250u) | 2u));
        PE_StoreU8(GA_D_8009D25C, 6u);
        return 0;
    }
    if (phase == 6u) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u || PE_LoadU8(aya + 0x252u) != 0u)
            return 0;
        /* 30584 / 77CF4 / 6DE80 camera deferred. */
        PE_StoreU16(GA_D_8009CE4C, 30u);
        PE_StoreU8(GA_D_8009D25C, 7u);
        return 0;
    }
    if (phase == 7u) {
        int16_t timer;

        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u &&
            PE_LoadU8(aya + 0x0Fu) == (uint8_t)PE_LoadU16(aya + 0x1Au)) {
            func_8001A680_command_cut(aya, 7u);
            PE_StoreU32(aya + 0x14u,
                        (uint32_t)PE_LoadU8(aya + 0x0Fu) << 15);
        }
        timer = (int16_t)PE_LoadU16(GA_D_8009CE4C);
        if (timer != 0) {
            PE_StoreU16(GA_D_8009CE4C, (uint16_t)(timer - 1));
            return 0;
        }
        if (aya != 0u) {
            PE_StoreU8(aya + 0x252u, 1u);
            PE_StoreU16(aya + 0x250u,
                        (uint16_t)(PE_LoadU16(aya + 0x250u) | 4u));
        }
        PE_StoreU8(GA_D_8009D25C, 8u);
        return 0;
    }
    if (phase == 8u) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya == 0u)
            return 0;
        if (PE_LoadU8(aya + 0x0Fu) != (uint8_t)PE_LoadU16(aya + 0x1Au))
            return 0;
        PE_StoreU16(aya + 0x250u,
                    (uint16_t)(PE_LoadU16(aya + 0x250u) | 0x20u));
        cmd = ((unsigned int)(int)(int8_t)PE_LoadU8(GA_D_8009CE48) << 1) + 8u;
        func_8001A680_command_cut(aya, cmd & 0xFFFEu);
        PE_StoreU8(GA_D_8009D25C, 9u);
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

/* Original target pulse reset. Command completion is in26824_port.c. */
#define GA_D_8009D2B0 0x8009D2B0u

void func_80026FD0(void)
{
    if ((int8_t)PE_LoadU8(GA_D_8009D2B0) == 0)
        return;
    PE_StoreU8(0x8009CE68u, 0x80u);
    PE_StoreU8(0x8009CE6Cu, (uint8_t)-8);
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
            sum = (uint16_t)((int)sum - ((int)rate * 2) / 5);
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
    hp = (int32_t)(PE_LoadU32(body + 0x10u) - (uint32_t)(int32_t)tick);
    PE_StoreU32(body + 0x10u, (uint32_t)hp);
    PE_StoreU32(body, word & 0xFFFFFC1Fu);
}

/* 36254 restores each of an actor's three script lists from saved PCs. */
void func_80036254(pe_addr_t actor)
{
    unsigned i;
    for (i=0;i<3;i++) {
        pe_addr_t task=PE_LoadU32(actor+0xA0u+i*4u);
        for (;task;task=PE_LoadU32(task+0x24u)) {
            pe_addr_t pc=PE_LoadU32(task+4u);
            if (!pc) continue;
            PE_StoreU32(task,pc);PE_StoreU32(task+0x10u,1u);
            PE_StoreU16(task+8u,PE_LoadU16(task+8u)&0xFF9Fu);
        }
    }
}

static void pe_27d14_stop(pe_addr_t actor)
{
    PE_StoreU32(actor+0x68u,0u);PE_StoreU32(actor+0x6Cu,0u);PE_StoreU32(actor+0x70u,0u);
}

static void pe_27d14_release_status(pe_addr_t actor, pe_addr_t body)
{
    pe_addr_t action;
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x1000u);
    func_80036254(actor);
    action=PE_LoadU32(body+0x18u);
    if (action) {
        PE_StoreU8(action,4u);
        PE_StoreU32(body,PE_LoadU32(body)&0xC0FFFFFFu);
    }
}

static void pe_27d14_reaction(pe_addr_t actor, pe_addr_t body)
{
    int8_t kind=(int8_t)PE_LoadU8(body+5u);
    uint32_t flags=PE_LoadU32(body);
    if (!(flags&14u)) {
        if (kind) return;
        if (PE_LoadU8(actor+15u)==PE_LoadU16(actor+26u)) {
            if (PE_LoadU8(body+0xBCu)==2u && !(flags&0x1800u)) {
                PE_StoreU8(body+0xBCu,1u);
                func_8001A680_command_cut(actor,PE_LoadU8(body+0xBDu));
                if (PE_LoadU8(body+0xBEu)) PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x200u);
                PE_StoreU32(actor+0x14u,PE_LoadU32(body+0xC0u));
                PE_StoreU32(actor+0x18u,PE_LoadU32(body+0xC4u));
                PE_StoreU32(actor+0x1Cu,PE_LoadU32(body+0xC8u));
            } else func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(body+6u));
            PE_StoreU32(body,PE_LoadU32(body)&~BODY_HITMASK);
            if (!(PE_LoadU32(PE_LoadU32(GA_D_8009D278)+0x4Cu)&0x80000u) &&
                !(PE_LoadU32(body)&0x1800u)) {
                PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x1000u);
                func_80036254(actor);
            }
        } else if (PE_LoadU8(body+0xA4u)>=2u && PE_LoadU8(body+0xA5u)) {
            int32_t angle=(int16_t)PE_LoadU16(body+0xA8u);
            uint32_t distance=PE_LoadU16(body+0xA6u);
            PE_StoreU32(actor+40u,PE_LoadU32(actor+40u)+((distance*(uint32_t)func_80077CF4(angle))<<4u));
            PE_StoreU32(actor+48u,PE_LoadU32(actor+48u)+((distance*(uint32_t)func_80077DC4(angle))<<4u));
            PE_StoreU8(body+0xA5u,(uint8_t)(PE_LoadU8(body+0xA5u)-1u));
        }
        return;
    }
    PE_StoreU32(body,PE_LoadU32(body)&~BODY_HITMASK);
    if (!kind || (int32_t)PE_LoadU32(body+16u)<=0) return;
    if (kind==1) actor=PE_LoadU32(actor+0x18Cu);
    else if (kind==4) {
        pe_addr_t walk;
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
            pe_addr_t other=PE_LoadU32(walk);
            if (walk!=PE_LoadU32(GA_D_8009D254) && other && (int8_t)PE_LoadU8(other+5u)==4)
                PE_StoreU16(walk+0x250u,PE_LoadU16(walk+0x250u)|0x20u);
        }
        return;
    }
    PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|0x20u);
}

/* Full 27D14, including timed status release, hit flashes, damage text,
 * linked-actor defeat and movement guards. Authority: 120D8.s. */
void func_80027D14(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),walk;
    uint32_t flags,bits;
    int32_t delta;
    int8_t kind;
    if ((int32_t)PE_LoadU32(body+0x88u)<(int32_t)PE_LoadU32(body+16u))
        PE_StoreU32(body+16u,PE_LoadU32(body+0x88u));
    if (!(pe_d1a0()&0x100u)) {
        if (!PE_LoadU16(body+12u)) {
            flags=PE_LoadU32(body);
            if (flags&14u) {
                flags=(flags&~14u)|((((flags>>1u)-1u)&7u)<<1u);PE_StoreU32(body,flags);
                if (!(flags&0x180Eu)) pe_27d14_release_status(actor,body);
            }
            flags=PE_LoadU32(body);
            if (flags&0x1800u) {
                flags=(flags&~0x1800u)|((((flags>>11u)-1u)&3u)<<11u);PE_StoreU32(body,flags);
                if (!(flags&0x180Eu)) pe_27d14_release_status(actor,body);
            }
        }
        pe_27d14_atb(actor,body);pe_27d14_dot(body);
    }
    if (PE_LoadU32(body)&BODY_HITMASK) {
        (void)func_80027A08(actor);
        bits=PE_LoadU32(body)&BODY_HITMASK;
        if (bits==BODY_HIT) {
            func_80028574(actor);
            func_8006DCE4(PE_LoadU16(body+0xB0u),0u,(int16_t)PE_LoadU16(actor+0x268u),
                (int16_t)PE_LoadU16(actor+0x26Au),(int16_t)PE_LoadU16(actor+0x26Cu));
        } else if (bits==BODY_REACT) pe_27d14_reaction(actor,body);
        if ((int8_t)PE_LoadU8(body+5u)!=1) pe_27d14_stop(actor);
    }
    if ((PE_LoadU32(body)&14u) && (int32_t)PE_LoadU32(body+16u)>0) {
        pe_27d14_stop(actor);
        if (!(pe_d1a0()&0x100u))
            PE_StoreU16(actor+0x3Au,(uint16_t)(((int16_t)PE_LoadU16(actor+0x3Au)+128)%4096));
    }
    if (PE_LoadU32(body)&0x1800u) pe_27d14_stop(actor);
    if (PE_LoadU32(body)&0x400u) PE_StoreU32(body+16u,0xFFFFFFFFu);
    delta=(int32_t)(PE_LoadU32(body+20u)-PE_LoadU32(body+16u));
    if (delta) {
        PE_StoreU16(body+0xD0u,(uint16_t)(delta<0?0u-(uint32_t)delta:(uint32_t)delta));
        PE_StoreU8(body+0xD7u,delta<0?1u:(PE_LoadU32(body)&0x38000u)==0x18000u?2u:0u);
        PE_StoreU16(body+0xD2u,PE_LoadU16(actor+0x218u));
        PE_StoreU16(body+0xD4u,(uint16_t)(PE_LoadU16(actor+0x21Au)-20u));
        PE_StoreU8(body+0xD6u,30u);
    }
    if (PE_LoadU8(body+0xD6u)) {
        func_80032B0C(1u,body+0xD0u);
        PE_StoreU8(body+0xD6u,(uint8_t)(PE_LoadU8(body+0xD6u)-1u));
    }
    if ((int32_t)PE_LoadU32(body+16u)<=0 && !(pe_d1a0()&0x100u)) {
        kind=(int8_t)PE_LoadU8(body+5u);
        if (kind==1 && !(PE_LoadU32(body)&BODY_HITMASK)) {
            PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x10u);func_8002F970(actor);
        }
        if (kind==4) {
            if (!(PE_LoadU32(body)&BODY_HITMASK)) { pe_27d14_stop(actor);func_80028E94(actor); }
        } else if (kind!=1 && kind!=3) { pe_27d14_stop(actor);func_80028E94(actor); }
        else {
            for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
                pe_addr_t other=PE_LoadU32(walk);
                int8_t k;
                if (!other || walk==PE_LoadU32(GA_D_8009D254)) continue;
                k=(int8_t)PE_LoadU8(other+5u);
                if (k && k!=2 && k!=4 && (int32_t)PE_LoadU32(other+16u)>0) break;
            }
            if (!walk) {
                if (kind==1) {
                    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x10u);func_8002F970(actor);
                    actor=PE_LoadU32(actor+0x18Cu);
                }
                pe_27d14_stop(actor);func_80028E94(actor);
            }
        }
    }
    PE_StoreU32(body+20u,PE_LoadU32(body+16u));
}
