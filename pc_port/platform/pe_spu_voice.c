/*
 * Guest voice → SPU register bridge + Psy-Q SpuSetCommonAttr / SpuSetVoiceAttr.
 *
 * PE_SpuVoice_ApplyPending publishes guest voice+0xF4 pending bits.
 * AUD1-E8: func_800878F0 (Akao_WriteVoiceParam) + func_80089F08 (ENVX).
 * func_80088344 (Akao_SetVoiceKeyOff) remains named-asm / host stub elsewhere.
 */
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

/* khasinski AKAO_VOICE_PARAM_* bit layout (VoiceParams.flags at voice+0xF4). */
#define VP_VOLUME_L          0x0001u
#define VP_VOLUME_R          0x0002u
#define VP_VOLUME            (VP_VOLUME_L | VP_VOLUME_R)
#define VP_PITCH             0x0010u
#define VP_START_ADDR        0x0080u
#define VP_ADSR_ATTACK_MODE  0x0100u
#define VP_ADSR_SUSTAIN_MODE 0x0200u
#define VP_ADSR_RELEASE_MODE 0x0400u
#define VP_ADSR_ATTACK_RATE  0x0800u
#define VP_ADSR_DECAY_RATE   0x1000u
#define VP_ADSR_SUSTAIN_RATE 0x2000u
#define VP_ADSR_RELEASE_RATE 0x4000u
#define VP_ADSR_SUSTAIN_LVL  0x8000u
#define VP_LOOP_ADDR         0x10000u
#define VP_ADSR_ATTACK       (VP_ADSR_ATTACK_MODE | VP_ADSR_ATTACK_RATE)
#define VP_ADSR_SUSTAIN      (VP_ADSR_SUSTAIN_MODE | VP_ADSR_SUSTAIN_RATE)
#define VP_ADSR_RELEASE      (VP_ADSR_RELEASE_MODE | VP_ADSR_RELEASE_RATE)
#define VP_ADSR_DECAY_SUSTAIN (VP_ADSR_DECAY_RATE | VP_ADSR_SUSTAIN_LVL)

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

/* SpuGetVoiceEnvelope — func_80089F08 (ENVX at voice*0x10+0xC). */
void func_80089F08(uint32_t voice_index, pe_addr_t env_out)
{
    uint16_t env;
    if (voice_index >= 24u)
        return;
    env = spu_half_load(voice_index * 0x10u + 0xCu);
    if (env_out >= 0x80000000u && env_out < 0x80200000u)
        PE_StoreU16(env_out, env);
}

static void spu_set_pitch(uint32_t index, uint32_t value)
{
    spu_half_store(index * 0x10u + 4u, (uint16_t)(value & 0xFFFFu));
}

static void spu_set_start(uint32_t index, uint32_t value)
{
    spu_half_store(index * 0x10u + 6u, (uint16_t)((value >> 3) & 0xFFFFu));
}

static void spu_set_loop(uint32_t index, uint32_t value)
{
    spu_half_store(index * 0x10u + 0xEu, (uint16_t)((value >> 3) & 0xFFFFu));
}

static void spu_set_adsr_attack(uint32_t index, uint32_t rate, uint32_t mode)
{
    uint32_t reg = index * 0x10u + 8u;
    uint16_t cur = spu_half_load(reg);
    uint32_t value = ((mode >> 2) << 15) | ((rate & 0xFFu) << 8);
    spu_half_store(reg, (uint16_t)((cur & 0xFFu) | value));
}

static void spu_set_adsr_decay(uint32_t index, uint32_t rate)
{
    uint32_t reg = index * 0x10u + 8u;
    uint16_t cur = spu_half_load(reg);
    spu_half_store(reg, (uint16_t)((cur & 0xFF0Fu) | ((rate & 0xFu) << 4)));
}

static void spu_set_adsr_sustain_level(uint32_t index, uint32_t level)
{
    uint32_t reg = index * 0x10u + 8u;
    uint16_t cur = spu_half_load(reg);
    spu_half_store(reg, (uint16_t)((cur & 0xFFF0u) | (level & 0xFu)));
}

static void spu_set_adsr_sustain_rate(uint32_t index, uint32_t rate, uint32_t mode)
{
    uint32_t reg = index * 0x10u + 0xAu;
    uint16_t cur = spu_half_load(reg);
    uint32_t value = ((mode >> 1) << 14) | ((rate & 0x7Fu) << 6);
    spu_half_store(reg, (uint16_t)((cur & 0x3Fu) | value));
}

static void spu_set_adsr_release(uint32_t index, uint32_t rate, uint32_t mode)
{
    uint32_t reg = index * 0x10u + 0xAu;
    uint16_t cur = spu_half_load(reg);
    uint32_t value = ((mode >> 2) << 5) | (rate & 0x1Fu);
    spu_half_store(reg, (uint16_t)((cur & 0xFFC0u) | value));
}

/*
 * Akao_WriteVoiceParam — func_800878F0.
 * params points at AkaoVoiceParams (guest): flags@+4, start@+8, loop@+C,
 * ADSR modes/rates, pitch@+0x1C, volumes@+0x28. Clears handled flag bits.
 */
void func_800878F0(int voice_index, pe_addr_t params)
{
    uint32_t flags;
    uint32_t cur;

    if (voice_index < 0 || voice_index >= 24 || !params)
        return;
    flags = PE_LoadU32(params + 4u);
    if (!flags)
        return;

    if (flags & VP_PITCH) {
        spu_set_pitch((uint32_t)voice_index, PE_LoadU16(params + 0x1Cu));
        cur = PE_LoadU32(params + 4u) & ~VP_PITCH;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_VOLUME) {
        func_80087798((uint32_t)voice_index, (uint32_t)(int16_t)PE_LoadU16(params + 0x28u),
                      (uint32_t)(int16_t)PE_LoadU16(params + 0x2Au));
        cur = PE_LoadU32(params + 4u) & ~VP_VOLUME;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_START_ADDR) {
        spu_set_start((uint32_t)voice_index, PE_LoadU32(params + 8u));
        cur = PE_LoadU32(params + 4u) & ~VP_START_ADDR;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_LOOP_ADDR) {
        spu_set_loop((uint32_t)voice_index, PE_LoadU32(params + 0xCu));
        cur = PE_LoadU32(params + 4u) & ~VP_LOOP_ADDR;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_ADSR_SUSTAIN) {
        spu_set_adsr_sustain_rate((uint32_t)voice_index, PE_LoadU16(params + 0x24u),
                                  (uint32_t)PE_LoadU32(params + 0x14u));
        cur = PE_LoadU32(params + 4u) & ~VP_ADSR_SUSTAIN;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_ADSR_ATTACK) {
        spu_set_adsr_attack((uint32_t)voice_index, PE_LoadU16(params + 0x1Eu),
                            (uint32_t)PE_LoadU32(params + 0x10u));
        cur = PE_LoadU32(params + 4u) & ~VP_ADSR_ATTACK;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_ADSR_RELEASE) {
        spu_set_adsr_release((uint32_t)voice_index, PE_LoadU16(params + 0x26u),
                             (uint32_t)PE_LoadU32(params + 0x18u));
        cur = PE_LoadU32(params + 4u) & ~VP_ADSR_RELEASE;
        PE_StoreU32(params + 4u, cur);
        if (!cur)
            return;
        flags = cur;
    }
    if (flags & VP_ADSR_DECAY_SUSTAIN) {
        spu_set_adsr_decay((uint32_t)voice_index, PE_LoadU16(params + 0x20u));
        spu_set_adsr_sustain_level((uint32_t)voice_index, PE_LoadU16(params + 0x22u));
    }
    PE_StoreU32(params + 4u, 0);
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
    idx = guest_voice_index(voice);
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
    /* AKAO_VOICE_PARAM_PITCH = 1<<4 — AUD1-E6 host publish. */
    if (flags & 0x10u) {
        spu_half_store(reg + 4u, (uint16_t)(PE_LoadU32(voice + 0x44u) >> 16));
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
    if (flags & 0x400u) {
        spu_half_store(reg + 6u, (uint16_t)(PE_LoadU32(voice + 0xF8u) >> 3));
        applied |= 0x400u;
    }
    /*
     * AUD1-E8: ADSR modes/rates/loop via WriteVoiceParam (VoiceParams at
     * voice+0xF0). Excluded bit conflicts with this host bridge:
     *   0x80  host legacy pitch (khasinski START_ADDR)
     *   0x400 host start publish (khasinski RELEASE_MODE)
     *   0x1000/0x2000 host key-on/off (khasinski DECAY/SUSTAIN_RATE)
     */
    {
        uint32_t wp = flags & (0x100u | 0x200u | 0x800u | 0x4000u | 0x8000u |
                               0x10000u);
        if (wp) {
            uint32_t saved = flags;
            PE_StoreU32(voice + 0xF4u, wp);
            func_800878F0(idx, voice + 0xF0u);
            applied |= wp;
            flags = (saved & ~wp) | PE_LoadU32(voice + 0xF4u);
            PE_StoreU32(voice + 0xF4u, flags);
        }
    }
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
    if (flags & 0x1FF80u)
        applied |= flags & 0x1FF80u;
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
