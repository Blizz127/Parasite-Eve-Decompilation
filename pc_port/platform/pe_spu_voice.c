/*
 * Guest voice → SPU register bridge + Psy-Q SpuSetCommonAttr / SpuSetVoiceAttr.
 *
 * PE_SpuVoice_ApplyPending is the host stand-in for pending voice+0xF4 publish
 * (previously mislabeled as func_80085F74). Real func_80085F74 is SpuSetCommonAttr.
 */
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

static int guest_voice_index(pe_addr_t voice)
{
    if (voice >= 0x800BC000u && voice < 0x800BC000u + 12u * 0x11Cu)
        return (int)((voice - 0x800BC000u) / 0x11Cu);
    if (voice >= 0x800B8AC0u && voice < 0x800B8AC0u + 24u * 0x11Cu)
        return (int)((voice - 0x800B8AC0u) / 0x11Cu) + 12;
    return -1;
}

static void spu_half_store(uint32_t offset, uint16_t value)
{
    pe_addr_t base = PE_LoadU32(0x8009B3FCu);
    if (!base || (base & 0x1FFFFFFFu) == 0x1F801C00u) {
        PE_SpuRegister_StoreU16(offset, value);
        return;
    }
    if (base >= 0x80000000u)
        PE_StoreU16(base + offset, value);
}

static uint16_t spu_half_load(uint32_t offset)
{
    pe_addr_t base = PE_LoadU32(0x8009B3FCu);
    if (!base || (base & 0x1FFFFFFFu) == 0x1F801C00u)
        return PE_SpuRegister_LoadU16(offset);
    if (base >= 0x80000000u)
        return PE_LoadU16(base + offset);
    return 0;
}

void func_80087798(uint32_t voice_index, uint32_t vol_left, uint32_t vol_right)
{
    uint32_t reg = voice_index * 0x10u;
    spu_half_store(reg, (uint16_t)(vol_left & 0x7FFFu));
    spu_half_store(reg + 2u, (uint16_t)(vol_right & 0x7FFFu));
}

/* Retail SpuSetVoiceAttr — func_800862F4. */
void func_800862F4(int voice, uint16_t left, uint16_t right, int16_t left_mode,
                   uint16_t right_mode)
{
    unsigned left_flags = 0;
    unsigned right_flags = 0;
    unsigned i;
    uint32_t half_index;

    if (voice < 0 || voice >= 24)
        return;

    left &= 0x7FFFu;
    half_index = (uint32_t)voice * 8u; /* halfword index into the register window */
    switch ((int16_t)(left_mode - 1)) {
    case 0: left_flags = 0x8000; break;
    case 1: left_flags = 0x9000; break;
    case 2: left_flags = 0xA000; break;
    case 3: left_flags = 0xB000; break;
    case 4: left_flags = 0xC000; break;
    case 5: left_flags = 0xD000; break;
    case 6: left_flags = 0xE000; break;
    }
    spu_half_store(half_index * 2u, (uint16_t)(left | left_flags));
    left = right & 0x7FFFu;
    switch ((int16_t)(right_mode - 1)) {
    case 0: right_flags = 0x8000; break;
    case 1: right_flags = 0x9000; break;
    case 2: right_flags = 0xA000; break;
    case 3: right_flags = 0xB000; break;
    case 4: right_flags = 0xC000; break;
    case 5: right_flags = 0xD000; break;
    case 6: right_flags = 0xE000; break;
    }
    spu_half_store(half_index * 2u + 2u, (uint16_t)(left | right_flags));
    for (i = 0; i < 2u; i++) {
    }
}

/* Retail SpuSetCommonAttr — func_80085F74 (writes common regs from attr block). */
void func_80085F74(pe_addr_t attr)
{
    uint32_t mask = PE_LoadU32(attr);
    int all = mask == 0u;
    uint16_t left = 0;
    uint16_t right = 0;
    uint32_t mode = 0;

    if (all || (mask & 1u)) {
        if (all || (mask & 4u)) {
            int16_t lm = (int16_t)PE_LoadU16(attr + 8u);
            switch (lm) {
            case 1: mode = 0x8000; break;
            case 2: mode = 0x9000; break;
            case 3: mode = 0xA000; break;
            case 4: mode = 0xB000; break;
            case 5: mode = 0xC000; break;
            case 6: mode = 0xD000; break;
            case 7: mode = 0xE000; break;
            case 0:
            default:
                left = PE_LoadU16(attr + 4u);
                mode = 0;
                break;
            }
        } else {
            left = PE_LoadU16(attr + 4u);
            mode = 0;
        }
        if (mode) {
            int16_t value = (int16_t)PE_LoadU16(attr + 4u);
            left = value > 0x7F ? 0x7F : value < 0 ? 0 : PE_LoadU16(attr + 4u);
        }
        spu_half_store(0x180u, (uint16_t)((left & 0x7FFFu) | mode));
    }
    if (all || (mask & 2u)) {
        mode = 0;
        if (all || (mask & 8u)) {
            int16_t rm = (int16_t)PE_LoadU16(attr + 0xAu);
            switch (rm) {
            case 1: mode = 0x8000; break;
            case 2: mode = 0x9000; break;
            case 3: mode = 0xA000; break;
            case 4: mode = 0xB000; break;
            case 5: mode = 0xC000; break;
            case 6: mode = 0xD000; break;
            case 7: mode = 0xE000; break;
            case 0:
            default:
                right = PE_LoadU16(attr + 6u);
                mode = 0;
                break;
            }
        } else {
            right = PE_LoadU16(attr + 6u);
            mode = 0;
        }
        if (mode) {
            int16_t value = (int16_t)PE_LoadU16(attr + 6u);
            right = value > 0x7F ? 0x7F : value < 0 ? 0 : PE_LoadU16(attr + 6u);
        }
        spu_half_store(0x182u, (uint16_t)((right & 0x7FFFu) | mode));
    }
    if (all || (mask & 0x40u))
        spu_half_store(0x1B0u, PE_LoadU16(attr + 0x10u));
    if (all || (mask & 0x80u))
        spu_half_store(0x1B2u, PE_LoadU16(attr + 0x12u));
    if (all || (mask & 0x400u))
        spu_half_store(0x1B4u, PE_LoadU16(attr + 0x1Cu));
    if (all || (mask & 0x800u))
        spu_half_store(0x1B6u, PE_LoadU16(attr + 0x1Eu));
    if (all || (mask & 0x100u)) {
        uint16_t cnt = spu_half_load(0x1AAu);
        if (!PE_LoadU32(attr + 0x14u))
            cnt = (uint16_t)(cnt & ~4u);
        else
            cnt = (uint16_t)(cnt | 4u);
        spu_half_store(0x1AAu, cnt);
    }
    if (all || (mask & 0x200u)) {
        uint16_t cnt = spu_half_load(0x1AAu);
        if (!PE_LoadU32(attr + 0x18u))
            cnt = (uint16_t)(cnt & ~1u);
        else
            cnt = (uint16_t)(cnt | 1u);
        spu_half_store(0x1AAu, cnt);
    }
    if (all || (mask & 0x1000u)) {
        uint16_t cnt = spu_half_load(0x1AAu);
        if (!PE_LoadU32(attr + 0x20u))
            cnt = (uint16_t)(cnt & ~8u);
        else
            cnt = (uint16_t)(cnt | 8u);
        spu_half_store(0x1AAu, cnt);
    }
    if (all || (mask & 0x2000u)) {
        uint16_t cnt = spu_half_load(0x1AAu);
        if (!PE_LoadU32(attr + 0x24u))
            cnt = (uint16_t)(cnt & ~2u);
        else
            cnt = (uint16_t)(cnt | 2u);
        spu_half_store(0x1AAu, cnt);
    }
}

void PE_SpuVoice_ApplyPending(pe_addr_t voice)
{
    uint32_t flags, applied = 0;
    int idx;
    uint32_t reg;

    if (PE_LoadU32(voice) < 1u)
        return;
    flags = PE_LoadU32(voice + 0xF4u);
    if (!flags)
        return;
    /* Prefer sequencer-assigned HW index (+0xF0) from StepVoiceNote. */
    {
        uint32_t hw = PE_LoadU32(voice + 0xF0u);
        if (hw < 24u)
            idx = (int)hw;
        else
            idx = guest_voice_index(voice);
    }
    if (idx < 0 || idx >= 24)
        return;
    reg = (uint32_t)idx * 0x10u;

    if (flags & 0x1u) {
        func_80087798((uint32_t)idx, PE_LoadU16(voice + 0x76u),
                      PE_LoadU16(voice + 0x78u));
        applied |= 0x1u;
    }
    if (flags & 0x2u) {
        func_80087798((uint32_t)idx, PE_LoadU16(voice + 0x76u),
                      PE_LoadU16(voice + 0x78u));
        applied |= 0x2u;
    }
    /* AUD1-E6: pitch pending (0x10) from 8D844 / 87AA8 LFO / pitch slides. */
    if (flags & 0x10u) {
        uint32_t base = PE_LoadU32(voice + 0x30u);
        uint32_t slide = PE_LoadU32(voice + 0x34u);
        int32_t lfo = (int16_t)PE_LoadU16(voice + 0xE8u);
        uint16_t out;
        if ((base & 0xFFFF0000u) != 0u || slide != 0u)
            out = (uint16_t)((int32_t)((base + slide) >> 16) + lfo);
        else
            out = (uint16_t)((int32_t)(base & 0xFFFFu) + lfo);
        spu_half_store(reg + 4u, out);
        applied |= 0x10u;
    }
    if (flags & 0x40u) {
        spu_half_store(reg + 4u, (uint16_t)(PE_LoadU32(voice + 0x44u) >> 16));
        applied |= 0x40u;
    }
    if (flags & 0x80u) {
        spu_half_store(reg + 4u, (uint16_t)(PE_LoadU32(voice + 0x44u) >> 16));
        applied |= 0x80u;
    }
    /* Retail START_ADDR is 0x80; host bridge also accepted 0x400 historically. */
    if (flags & (0x80u | 0x400u)) {
        spu_half_store(reg + 6u, (uint16_t)(PE_LoadU32(voice + 0xF8u) >> 3));
        applied |= flags & (0x80u | 0x400u);
    }
    /* AUD1-E8: compose ADSR1/ADSR2 from voice+0xF0 overlay when ADSR bits set. */
    if (flags & 0x1FF80u) {
        pe_addr_t p = voice + 0xF0u;
        uint16_t adsr1 = PE_SpuRegister_LoadU16(reg + 8u);
        uint16_t adsr2 = PE_SpuRegister_LoadU16(reg + 0xAu);
        if (flags & 0x900u) {
            uint32_t rate = PE_LoadU16(p + 0x1Eu);
            uint32_t mode = PE_LoadU32(p + 0x10u);
            adsr1 = (uint16_t)((adsr1 & 0x00FFu) |
                               (((mode >> 2) << 15) | (rate << 8)));
        }
        if (flags & 0x1000u) {
            uint32_t rate = PE_LoadU16(p + 0x20u);
            adsr1 = (uint16_t)((adsr1 & 0xFF0Fu) | (rate << 4));
        }
        if (flags & 0x8000u) {
            uint32_t level = PE_LoadU16(p + 0x22u);
            adsr1 = (uint16_t)((adsr1 & 0xFFF0u) | (level & 0xFu));
        }
        if (flags & 0x2200u) {
            uint32_t rate = PE_LoadU16(p + 0x24u);
            uint32_t mode = PE_LoadU32(p + 0x14u);
            adsr2 = (uint16_t)((adsr2 & 0x3Fu) |
                               (((mode >> 1) << 14) | (rate << 6)));
        }
        if (flags & 0x4400u) {
            uint32_t rate = PE_LoadU16(p + 0x26u);
            uint32_t mode = PE_LoadU32(p + 0x18u);
            adsr2 = (uint16_t)((adsr2 & 0xFFC0u) |
                               (((mode >> 2) << 5) | (rate & 0x1Fu)));
        }
        /* Legacy packed halfword shortcut when only 0x800 set. */
        if ((flags & 0x800u) && !(flags & 0x900u)) {
            adsr1 = PE_LoadU16(voice + 0x10Eu);
            adsr2 = PE_LoadU16(voice + 0x110u);
        }
        spu_half_store(reg + 8u, adsr1);
        spu_half_store(reg + 0xAu, adsr2);
        applied |= flags & 0x1FF80u;
    }
    /* Host bridge: bit 0x1000 still keys on (ProcessVoiceQueue path separate). */
    if ((flags & 0x2000u) && !(flags & 0x1000u)) {
        if (idx < 16)
            spu_half_store(0x18Au, (uint16_t)(spu_half_load(0x18Au) | (1u << idx)));
        else
            spu_half_store(0x18Cu,
                           (uint16_t)(spu_half_load(0x18Cu) | (1u << (idx - 16))));
        applied |= 0x2000u;
    }
    if (flags & 0x1000u) {
        if (idx < 16)
            spu_half_store(0x188u, (uint16_t)(spu_half_load(0x188u) | (1u << idx)));
        else
            spu_half_store(0x18Au,
                           (uint16_t)(spu_half_load(0x18Au) | (1u << (idx - 16))));
        applied |= 0x1000u;
    }
    PE_StoreU32(voice + 0xF4u, flags & ~applied);
}

void PE_SpuScore_ApplyDirtyVoices(void)
{
    uint32_t dirty = PE_LoadU32(0x8009D2C4u);
    unsigned i;

    if (!(dirty & 0x100u))
        return;

    {
        uint32_t active = PE_LoadU32(0x800BCD50u);
        for (i = 0; i < 12u; i++) {
            if (active & (0x1000u << i))
                PE_SpuVoice_ApplyPending(0x800BC000u + i * 0x11Cu);
        }
    }
    {
        pe_addr_t state = PE_LoadU32(0x8009D2C8u);
        if (state) {
            uint32_t selected = PE_LoadU32(state + 4u);
            for (i = 0; i < 24u; i++) {
                if (selected & (1u << i))
                    PE_SpuVoice_ApplyPending(0x800B8AC0u + i * 0x11Cu);
            }
        }
    }
    PE_StoreU32(0x8009D2C4u, dirty & ~0x100u);
}
