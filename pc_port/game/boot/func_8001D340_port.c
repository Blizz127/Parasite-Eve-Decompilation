/*
 * PE-BTL98/99 — live 1D340 path through 1F704 and 1F814.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. Retail BTL83
 * capture: 299CC jal 1D340 @ 0x8002A4FC a0=1, then 1F704
 * ra=0x8001F5F0 stores HP 40→39. No matching src/ C.
 *
 * 1D340 this cut:
 *   a0!=0 ATB: record+0x10 += +0x24 (delay-slot sh at 1D3A4)
 *   plus the 0xC0/0x100 speed adjust. Walk D20C skipping Aya
 *   and +0x98 bit 0x10. record+0x4C bit 0x4000 (ROM gate at
 *   1E780) jals 1F4D4(actor). Death is not at 1F4D4.
 *   Authentic 1F078 / 1F080 is after the 1D340 prefix
 *   (HUD sb storm, 21D4C, 6F6D4, …) and is not this cut.
 *
 * 1F4D4 this cut: through 1F704, then if remaining HP!=0
 * jal 1F814(actor) and maybe sw actor → D1D0. 20288 stays
 * deferred.
 *
 * 1F814: Aya+0x0E in 6..15 takes jtbl 0x800106E4
 * (D29A/D29B/gp+0x528/D29C). Default path jals 305C8 then
 * 1A680(D254, facing cmd) then 6DE80(0x46A, 0, Aya+0x2A/2E/32).
 * 6DED4 → 6DFA8 / 6DF50 stay fail-closed.
 *
 * 71A54 is BIOS A(2Fh) rand (3-word trampoline, SHA-256
 * db6da112…cffa). Used after s0 is already computed in the
 * jal delay slot; first retail delta is s0=1.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D1D0 0x8009D1D0u
#define GA_D_8009D298 0x8009D298u /* gp+0x528 */
#define GA_D_8009D29A 0x8009D29Au
#define GA_D_8009D29B 0x8009D29Bu
#define GA_D_8009D29C 0x8009D29Cu

static uint32_t s_bios_rand_seed = 1u;

unsigned int func_80071A54(void)
{
    s_bios_rand_seed = s_bios_rand_seed * 0x41C64E6Du + 0x3039u;
    return (s_bios_rand_seed >> 16) & 0x7FFFu;
}

static uint32_t pe_u32_div20(uint32_t n)
{
    return (uint32_t)(((uint64_t)n * 0xCCCCCCDFull) >> 34);
}

static uint32_t pe_u32_mul3_div10(uint32_t n)
{
    return (uint32_t)(((uint64_t)(n * 3u) * 0x66666667ull) >> 34);
}

int32_t func_800305C8(pe_addr_t attacker, pe_addr_t target)
{
    int32_t dx;
    int32_t dz;
    int32_t ang;
    int32_t wrap;
    int32_t base;

    dx = (int32_t)PE_LoadU32(attacker + 0x28u) -
         (int32_t)PE_LoadU32(target + 0x28u);
    dz = (int32_t)PE_LoadU32(attacker + 0x30u) -
         (int32_t)PE_LoadU32(target + 0x30u);
    ang = 2048 - func_80079FB4(dx, dz);
    ang = (int32_t)(int16_t)ang;
    ang += (int32_t)(int16_t)PE_LoadU16(target + 0x3Au);
    wrap = ang;
    if (ang < 0)
        wrap = ang + 4095;
    base = (wrap >> 12) << 12;
    return (int32_t)(int16_t)(ang - base);
}

int32_t func_8001F814(pe_addr_t actor)
{
    pe_addr_t aya;
    pe_addr_t rec;
    uint8_t kind;
    uint8_t lo;
    uint32_t flags;
    int32_t s0;
    int32_t angle;
    unsigned int hit_cmd;

    s0 = 0;
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya == 0u)
        return 0;

    kind = PE_LoadU8(aya + 0x0Eu);
    if (kind >= 6u && kind <= 15u) {
        lo = PE_LoadU8(aya + 0x0Fu);
        PE_StoreU8(GA_D_8009D29A, kind);
        PE_StoreU16(GA_D_8009D298, 1u);
        PE_StoreU8(GA_D_8009D29B, lo);
        if (kind == 7u || kind == 9u || kind == 11u)
            PE_StoreU32(GA_D_8009D29C, (uint32_t)lo << 16);
        else
            PE_StoreU32(GA_D_8009D29C, PE_LoadU32(aya + 0x14u));
    }

    rec = PE_LoadU32(GA_D_8009D278);
    flags = (rec != 0u) ? PE_LoadU32(rec + 0x4Cu) : 0u;
    if ((flags & 0x00012000u) != 0u || actor == 0u)
        return 0;
    s0 = func_800305C8(actor, aya);
    angle = (int32_t)(int16_t)s0;
    if (angle < 512)
        hit_cmd = 0u;
    else if (angle < 1536)
        hit_cmd = 2u;
    else if (angle < 2560)
        hit_cmd = 1u;
    else if (angle < 3584)
        hit_cmd = 3u;
    else
        hit_cmd = 0u;
    func_8001A680_command_cut(aya, hit_cmd);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u) {
        func_8006DE80(0x46A, 0,
                      (int)(int16_t)PE_LoadU16(aya + 0x2Au),
                      (int)(int16_t)PE_LoadU16(aya + 0x2Eu),
                      (int)(int16_t)PE_LoadU16(aya + 0x32u));
        flags = PE_LoadU32(aya + 0x98u);
        if (flags & 0x100u) {
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
            PE_StoreU16(GA_D_8009D298, 2u);
        }
    }
    return (int32_t)(int16_t)s0;
}

void func_8001F4D4(pe_addr_t actor)
{
    pe_addr_t rec;
    pe_addr_t body;
    pe_addr_t weapon;
    pe_addr_t tbl;
    pe_addr_t aya;
    uint32_t flags;
    uint32_t def;
    uint32_t a2;
    uint32_t kind;
    uint32_t word;
    int s0;
    int16_t hp;
    uint32_t rnd;
    uint32_t rem;
    uint32_t thresh;
    int64_t prod;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u || actor == 0u)
        return;
    body = PE_LoadU32(actor);
    if (body == 0u)
        return;
    weapon = PE_LoadU32(body + 0x18u);
    if (weapon == 0u)
        return;

    flags = PE_LoadU32(rec + 0x4Cu);
    def = pe_u32_div20(PE_LoadU16(rec + 0x20u));
    if (flags & 0x1000u)
        def = pe_u32_mul3_div10(def);
    if (flags & 0x100u)
        def = pe_u32_mul3_div10(def);

    kind = (PE_LoadU32(body) >> 21) & 7u;
    tbl = PE_LoadU32(rec + 0x6Cu);
    word = (tbl != 0u) ? PE_LoadU32(tbl) : 0u;
    if (kind < 3u) {
        uint8_t elem;

        a2 = word & 0x3FFu;
        elem = PE_LoadU8(weapon + 0x0Eu);
        if (elem == 0u)
            PE_StoreU8(weapon, 4u);
        else if (elem != 1u)
            PE_StoreU8(weapon, 3u);
    } else {
        a2 = (word >> 10) & 0x3FFu;
    }

    s0 = (int)PE_LoadU16(weapon + 0x0Cu) - (int)def - (int)a2;
    rnd = func_80071A54();
    /* 0x51EB851F signed /100; remainder vs (lbu+0x90)*(100-hi). */
    prod = (int64_t)(int32_t)rnd * (int64_t)(int32_t)0x51EB851F;
    rem = (uint32_t)rnd - (uint32_t)((int)(prod >> 37) * 100);
    thresh = ((uint32_t)PE_LoadU8(body + 0x90u) *
              (100u - ((word >> 20) & 0xFFu))) / 100u;
    if (rem < thresh) {
        s0 = (s0 * 3) / 2;
        PE_StoreU32(rec + 0x4Cu, flags | 0x8000u);
        flags |= 0x8000u;
    }

    if (flags & 0x200u)
        return;
    if ((int)PE_LoadU32(rec + 0x34u) > 0)
        s0 >>= 1;
    if (s0 <= 0)
        return;
    if (PE_LoadU8(weapon + 1u) == 10u)
        s0 >>= 1;
    hp = (int16_t)PE_LoadU16(rec + 0x0Cu);
    hp = (int16_t)(hp - (int16_t)s0);
    PE_StoreU16(rec + 0x0Cu, (uint16_t)hp);

    /* 1F708: remaining HP!=0 → 1F814; HP==0 skips to 1F7D8. */
    rec = PE_LoadU32(GA_D_8009D278);
    if (rec != 0u && (int16_t)PE_LoadU16(rec + 0x0Cu) != 0) {
        (void)func_8001F814(actor);
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u && (PE_LoadU32(aya + 0x98u) & 0x100u) == 0u)
            PE_StoreU32(GA_D_8009D1D0, actor);
    }
}

void func_8001D340(unsigned int a0)
{
    pe_addr_t rec;
    pe_addr_t aya;
    pe_addr_t actor;
    uint32_t flags;
    uint16_t atb;
    uint16_t spd;
    uint16_t sum;
    uint32_t c0;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;

    if ((a0 & 0xFFu) != 0u) {
        atb = PE_LoadU16(rec + 0x10u);
        spd = PE_LoadU16(rec + 0x24u);
        sum = (uint16_t)(atb + spd);
        flags = PE_LoadU32(rec + 0x4Cu);
        c0 = flags & 0xC0u;
        PE_StoreU16(rec + 0x10u, sum);
        if (c0 == 0x40u || c0 == 0x80u) {
            uint32_t v = (uint32_t)spd << 1u;
            int hi = (int)(((int64_t)(int32_t)v * (int64_t)(int32_t)0x66666667)
                           >> 32);
            int adj = (hi >> 1) - (v >> 31);
            PE_StoreU16(rec + 0x10u, (uint16_t)(sum - (uint16_t)adj));
        } else if (flags & 0x100u) {
            PE_StoreU16(rec + 0x10u, (uint16_t)(sum + (spd >> 1)));
        }
    }

    flags = PE_LoadU32(rec + 0x4Cu);
    if ((flags & 0x00800000u) != 0u || (flags & 0x00180000u) != 0u)
        return;

    aya = PE_LoadU32(GA_D_8009D254);
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        pe_addr_t body;
        pe_addr_t next;

        next = PE_LoadU32(actor + 4u);
        if (actor != aya &&
            (PE_LoadU32(actor + 0x98u) & 0x10u) == 0u) {
            body = PE_LoadU32(actor);
            if (body != 0u && PE_LoadU32(body + 0x18u) != 0u &&
                (flags & 0x4000u) != 0u) {
                func_8001F4D4(actor);
                break;
            }
        }
        actor = next;
    }
}
