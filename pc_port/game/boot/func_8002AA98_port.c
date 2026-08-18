/*
 * PE-BTL100 — 2A7F8 mode-3: 2AA98 / 2B29C case 0.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 2A7F8 is a compare chain, not a jump table:
 *   mode 1 → 25EE8 (not this cut)
 *   mode 2 → 2B0E8 (not this cut)
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
 * Later phases (fade, 6DE80 0x4AF, mode=-1 / 6A25C,
 * 77AC4, 32B0C, 27D14, 5409C) stay deferred.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009CE70 0x8009CE70u /* gp+0x100 timer */
#define GA_D_8009CE74 0x8009CE74u /* gp+0x104 phase */

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

void func_8002B29C(void)
{
    pe_addr_t aya;
    pe_addr_t actor;
    uint8_t phase;
    uint32_t flags;

    phase = PE_LoadU8(GA_D_8009CE74);
    if (phase >= 6u)
        return;
    if (phase != 0u)
        return;

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
