/*
 * PE-BTL105 — 27D14 enemy tick and 28E94 death phases.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 27D14: 534 words 0x80027D14..0x80028570. a0 = actor, *actor = body.
 * After 1D340, 299CC @ 0x8002A53C walks non-Aya bodies here.
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
#define GA_D_800A5D58 0x800A5D58u
#define SLOT_STRIDE   220u

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

    /* body&0x6000 → 27A08 / 28574 / 6DCE4 deferred. */
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
