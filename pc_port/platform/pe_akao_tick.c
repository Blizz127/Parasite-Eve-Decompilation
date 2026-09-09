/*
 * Host Akao_Tick (func_8008DB7C) — score sequencer tick.
 * Callees not yet ported are no-op stubs so the drain/reverb/pitch tail and
 * bank walk structure can run; PE_SpuScore_ApplyDirtyVoices still publishes
 * pending voice bits after this returns.
 */
#include "pe_sdk.h"
#include "pe_spu_voice.h"
#include "psx_compat.h"

void func_80089328(void) {}
void func_8008E8D0(pe_addr_t voice, uint32_t mask)
{
    (void)voice;
    (void)mask;
}
void func_80087AA8(pe_addr_t voice, uint32_t mask)
{
    (void)voice;
    (void)mask;
}
void func_80087FA0(pe_addr_t voice, uint32_t mask)
{
    (void)voice;
    (void)mask;
}
void func_8008D844(void) {}
void func_80089784(void) {}

/* Matched leaf already exists as host memcpy-style helper in some tests;
 * provide a PE-RAM word copy if not linked from elsewhere. */
void func_8008D820(pe_addr_t dst, pe_addr_t src, uint32_t nbytes)
{
    uint32_t words = nbytes >> 2;
    while (words--) {
        PE_StoreU32(dst, PE_LoadU32(src));
        dst += 4u;
        src += 4u;
    }
}

static uint32_t scale_tempo(uint32_t tempo, uint32_t factor)
{
    uint32_t product;
    if (!factor)
        return tempo;
    product = tempo * factor;
    if (factor < 0x80u)
        return tempo + (product >> 7);
    return product >> 8;
}

static void tick_bank(pe_addr_t base, uint32_t mask,
                      void (*update)(pe_addr_t, uint32_t), int stream_bank)
{
    uint32_t bit = stream_bank ? 0x1000u : 1u;
    pe_addr_t voice = base;

    while (mask) {
        if (mask & bit) {
            int run = 1;
            if (stream_bank && (PE_LoadU32(0x8009D2DCu) & 2u)) {
                if ((PE_LoadU32(voice + 0x2Cu) & 0x2000000u) == 0u)
                    run = 0;
            }
            if (run) {
                uint16_t t0 = (uint16_t)(PE_LoadU16(voice + 0x56u) - 1u);
                uint16_t t1 = (uint16_t)(PE_LoadU16(voice + 0x58u) - 1u);
                PE_StoreU16(voice + 0x56u, t0);
                PE_StoreU16(voice + 0x58u, t1);
                if (stream_bank)
                    PE_StoreU32(voice + 0x50u, PE_LoadU32(voice + 0x50u) + 1u);
                if (t0 == 0u) {
                    func_8008E8D0(voice, bit);
                } else if (t1 == 0u) {
                    if (stream_bank) {
                        PE_StoreU32(0x800BCD5Cu, PE_LoadU32(0x800BCD5Cu) | bit);
                        PE_StoreU32(0x800BCD58u, PE_LoadU32(0x800BCD58u) & ~bit);
                    } else {
                        pe_addr_t st = PE_LoadU32(0x8009D2C8u);
                        PE_StoreU32(st + 0x18u, PE_LoadU32(st + 0x18u) | bit);
                        PE_StoreU32(st + 0x14u, PE_LoadU32(st + 0x14u) & ~bit);
                    }
                }
                update(voice, bit);
            }
            mask ^= bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
    }
}

static void tick_slides(int mark_dirty)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    uint16_t slide = PE_LoadU16(st + 0x52u);
    if (slide) {
        PE_StoreU16(st + 0x52u, (uint16_t)(slide - 1u));
        PE_StoreU32(st + 0x20u, PE_LoadU32(st + 0x20u) + PE_LoadU32(st + 0x24u));
        st = PE_LoadU32(0x8009D2C8u);
    }
    slide = PE_LoadU16(st + 0x58u);
    if (slide) {
        PE_StoreU16(st + 0x58u, (uint16_t)(slide - 1u));
        PE_StoreU32(st + 0x40u, PE_LoadU32(st + 0x40u) + PE_LoadU32(st + 0x44u));
        if (mark_dirty)
            PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x80u);
        st = PE_LoadU32(0x8009D2C8u);
    }
    {
        uint16_t period = PE_LoadU16(st + 0x60u);
        if (period) {
            uint16_t count = (uint16_t)(PE_LoadU16(st + 0x62u) + 1u);
            PE_StoreU16(st + 0x62u, count);
            if (count == period) {
                uint16_t a = (uint16_t)(PE_LoadU16(st + 0x5Eu) + 1u);
                uint16_t b = PE_LoadU16(st + 0x5Cu);
                PE_StoreU16(st + 0x62u, 0);
                PE_StoreU16(st + 0x5Eu, a);
                if (a == b) {
                    uint16_t c = (uint16_t)(PE_LoadU16(st + 0x64u) + 1u);
                    PE_StoreU16(st + 0x5Eu, 0);
                    PE_StoreU16(st + 0x64u, c);
                    {
                        uint32_t left = PE_LoadU32(0x8009D22Cu);
                        if (left)
                            PE_StoreU32(0x8009D22Cu, left - 1u);
                    }
                }
            }
        }
    }
}

void func_8008DB7C(void)
{
    pe_addr_t st;
    uint32_t active, tempo, acc;

    func_80089328();

    st = PE_LoadU32(0x8009D2C8u);
    if (!st || st < 0x80000000u || st >= 0x80200000u) {
        if (!PE_LoadU32(0x8009D268u))
            func_8008CA84();
        func_8008D844();
        func_80089784();
        return;
    }

    active = PE_LoadU32(st + 4u);
    if (active) {
        tempo = PE_LoadU16(st + 0x22u);
        tempo = scale_tempo(tempo, PE_LoadU8(0x8009D2D2u));
        acc = PE_LoadU32(st + 0x28u) + tempo;
        PE_StoreU32(st + 0x28u, acc);
        if (!((acc & 0xFFFF0000u) == 0u && (PE_LoadU32(0x8009D2DCu) & 4u) == 0u)) {
            PE_StoreU32(st + 0x28u, acc & 0xFFFFu);
            do {
                tick_bank(0x800B8AC0u, PE_LoadU32(PE_LoadU32(0x8009D2C8u) + 4u),
                          func_80087AA8, 0);
                tick_slides(1);
            } while (PE_LoadU32(0x8009D22Cu));
        }
    }

    st = PE_LoadU32(0x8009D2C8u);
    if (PE_LoadU32(st + 0x6Cu)) {
        pe_addr_t primary = st;
        uint32_t t = PE_LoadU16(primary + 0x8Au);
        PE_StoreU32(0x8009D2C8u, primary + 0x68u);
        st = primary + 0x68u;
        t = scale_tempo(t, PE_LoadU8(0x8009D2D2u));
        acc = PE_LoadU32(st + 0x28u) + t;
        PE_StoreU32(st + 0x28u, acc);
        if (!((acc & 0xFFFF0000u) == 0u && (PE_LoadU32(0x8009D2DCu) & 4u) == 0u)) {
            PE_StoreU32(st + 0x28u, acc & 0xFFFFu);
            tick_bank(0x800BA560u, PE_LoadU32(st + 4u), func_80087AA8, 0);
            tick_slides(0);
        }
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }

    st = PE_LoadU32(0x8009D2C8u);
    if (!PE_LoadU32(st + 4u) && !PE_LoadU32(st + 0x1Cu) &&
        PE_LoadU32(st + 0x6Cu)) {
        func_8008D820(st + 0x68u, st, 0x68u);
        func_8008D820(0x800BA560u, 0x800B8AC0u, 0x1AA0u);
        st = PE_LoadU32(0x8009D2C8u);
        PE_StoreU16(st + 0xBCu, 0);
        PE_StoreU32(st + 0x6Cu, 0);
    }

    active = PE_LoadU32(0x800BCD50u);
    if (active) {
        acc = PE_LoadU32(0x800BCD68u) + PE_LoadU16(0x800BCD66u);
        PE_StoreU32(0x800BCD68u, acc);
        if (!((acc & 0xFFFF0000u) == 0u && (PE_LoadU32(0x8009D2DCu) & 4u) == 0u)) {
            PE_StoreU32(0x800BCD68u, acc & 0xFFFFu);
            tick_bank(0x800BC000u, active, func_80087FA0, 1);
        }
    }

    if (!PE_LoadU32(0x8009D268u))
        func_8008CA84();
    func_8008D844();
    func_80089784();
}
