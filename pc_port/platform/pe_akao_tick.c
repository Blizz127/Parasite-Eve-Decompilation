/*
 * Host Akao_Tick (func_8008DB7C) + score callees (AUD1-E3..E6 fold).
 *
 * Semantic PE-RAM ports from khasinski donors / candidates (not byte-match):
 *   87AA8 Spu_UpdateVoiceRegisters, 87FA0 stream twin (shared body),
 *   89328 Akao_ProcessVoiceQueue, 89724 Spu_VoiceMaskCompose,
 *   89784 key-off flush (khasinski Akao_SetVoicePitch body).
 * Still stubbed (no usable donor on the audio tip at fold time):
 *   8E8D0/8F0D0 sample bytecode, 8D844 reverb-load step, 8900C StepVoiceNote.
 * 89250 UpdateVoiceEnvelopes is hosted from khasinski voice_envelopes.c.
 */
#include "pe_sdk.h"
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

static void mark_audio_dirty(uint32_t bits)
{
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | bits);
}

static void spu_write_key_off(uint32_t mask)
{
    PE_SpuRegister_StoreU16(0x18Cu, (uint16_t)(mask & 0xFFFFu));
    PE_SpuRegister_StoreU16(0x18Eu, (uint16_t)((mask >> 16) & 0xFFFFu));
}

static void spu_write_key_on(uint32_t mask)
{
    PE_SpuRegister_StoreU16(0x188u, (uint16_t)(mask & 0xFFFFu));
    PE_SpuRegister_StoreU16(0x18Au, (uint16_t)((mask >> 16) & 0xFFFFu));
}

/* 8900C Akao_StepVoiceNote — still open (no donor on remote tip). */
static void func_8008900C(pe_addr_t voices, uint32_t active, uint32_t restart,
                          pe_addr_t key_on_out)
{
    (void)voices;
    (void)active;
    (void)restart;
    (void)key_on_out;
}

/* Akao_RemoveVoice — clear assigned_voice_index (== voice_index) → 0x18. */
static void akao_remove_voice(pe_addr_t base, uint32_t voice_index)
{
    unsigned i;
    pe_addr_t p = base + 0xF0u;
    for (i = 0; i < 0x18u; i++) {
        if (PE_LoadU32(p) == voice_index)
            PE_StoreU32(p, 0x18u);
        p += 0x11Cu;
    }
}

/* Akao_UpdateVoiceEnvelopes — func_80089250 (khasinski voice_envelopes.c). */
static void func_80089250(uint32_t blocked)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    uint32_t protected_mask = blocked;
    uint32_t voice_index;
    uint32_t bit;

    if (st && st >= 0x80000000u && st < 0x80200000u) {
        protected_mask |= (PE_LoadU32(st + 4u) & PE_LoadU32(st + 0xCu)) |
                          (PE_LoadU32(st + 0x6Cu) & PE_LoadU32(st + 0x74u));
    }

    bit = 1u;
    for (voice_index = 0; voice_index < 0x18u; voice_index++, bit <<= 1) {
        if (protected_mask & bit) {
            /* Protected voices keep full envelope in the guest table path;
             * host synth has no ENV table — skip. */
            continue;
        }
        {
            uint16_t env = PE_SpuRegister_LoadU16(voice_index * 0x10u + 0xCu);
            if (env == 0u) {
                akao_remove_voice(0x800B8AC0u, voice_index);
                akao_remove_voice(0x800BA560u, voice_index);
            }
        }
    }
}

/* Spu_VoiceMaskCompose — func_80089724 (khasinski matching C). */
void func_80089724(pe_addr_t track, pe_addr_t mask_out, uint32_t mask,
                   uint32_t mask_keep)
{
    uint32_t bit = 1u;
    do {
        if (mask & bit) {
            uint32_t idx = PE_LoadU32(track + 0xF0u);
            if (idx < 0x18u)
                PE_StoreU32(mask_out, PE_LoadU32(mask_out) | (1u << idx));
        }
        mask &= ~bit;
        track += 0x11Cu;
        bit <<= 1;
    } while (mask);
    PE_StoreU32(mask_out, PE_LoadU32(mask_out) & mask_keep);
}

/* Spu_UpdateVoiceRegisters — func_80087AA8 (khasinski candidate). */
static void update_voice_registers(pe_addr_t voice, uint32_t voice_mask)
{
    uint32_t old_value, new_value;
    int value, scaled;
    pe_addr_t table;
    pe_addr_t st;
    uint32_t f4_before = PE_LoadU32(voice + 0xF4u);

    if (PE_LoadU16(voice + 0x72u) != 0u) {
        old_value = PE_LoadU32(voice + 0x44u);
        new_value = old_value + PE_LoadU32(voice + 0x48u);
        PE_StoreU16(voice + 0x72u, (uint16_t)(PE_LoadU16(voice + 0x72u) - 1u));
        if ((new_value & 0xFFE00000u) != (old_value & 0xFFE00000u))
            PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
        PE_StoreU32(voice + 0x44u, new_value);
    }

    if (PE_LoadU16(voice + 0x60u) != 0u) {
        PE_StoreU16(voice + 0x60u, (uint16_t)(PE_LoadU16(voice + 0x60u) - 1u));
        PE_StoreU16(voice + 0x5Eu,
                    (uint16_t)(PE_LoadU16(voice + 0x5Eu) + PE_LoadU16(voice + 0xD6u)));
        PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
    }

    if (PE_LoadU16(voice + 0x6Eu) != 0u) {
        old_value = PE_LoadU16(voice + 0x6Cu);
        new_value = (uint32_t)(old_value + (int16_t)PE_LoadU16(voice + 0xD4u));
        PE_StoreU16(voice + 0x6Eu, (uint16_t)(PE_LoadU16(voice + 0x6Eu) - 1u));
        if ((new_value & 0x7F00u) != (old_value & 0x7F00u))
            PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
        PE_StoreU16(voice + 0x6Cu, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x74u) != 0u) {
        old_value = (uint32_t)(int16_t)PE_LoadU16(voice + 0xD8u);
        new_value = (uint32_t)((int32_t)old_value + (int16_t)PE_LoadU16(voice + 0xDAu));
        PE_StoreU16(voice + 0x74u, (uint16_t)(PE_LoadU16(voice + 0x74u) - 1u));
        if ((PE_LoadU32(voice + 0x38u) & 0x100u) != 0u &&
            ((new_value & 0xFF00u) != (old_value & 0xFF00u)))
            PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
        PE_StoreU16(voice + 0xD8u, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x78u) != 0u) {
        old_value = PE_LoadU16(voice + 0x76u);
        new_value = (uint32_t)(old_value + (int16_t)PE_LoadU16(voice + 0xDCu));
        PE_StoreU16(voice + 0x78u, (uint16_t)(PE_LoadU16(voice + 0x78u) - 1u));
        if ((new_value & 0xFF00u) != (old_value & 0xFF00u))
            PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
        PE_StoreU16(voice + 0x76u, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x8Au) != 0u)
        PE_StoreU16(voice + 0x8Au, (uint16_t)(PE_LoadU16(voice + 0x8Au) - 1u));
    if (PE_LoadU16(voice + 0x9Eu) != 0u)
        PE_StoreU16(voice + 0x9Eu, (uint16_t)(PE_LoadU16(voice + 0x9Eu) - 1u));

    st = PE_LoadU32(0x8009D2C8u);
    if (st && PE_LoadU16(voice + 0xBAu) != 0u) {
        uint16_t left = (uint16_t)(PE_LoadU16(voice + 0xBAu) - 1u);
        PE_StoreU16(voice + 0xBAu, left);
        if (left == 0u) {
            PE_StoreU32(st + 0x34u, PE_LoadU32(st + 0x34u) ^ voice_mask);
            mark_audio_dirty(0x100u);
        }
    }
    if (st && PE_LoadU16(voice + 0xBCu) != 0u) {
        uint16_t left = (uint16_t)(PE_LoadU16(voice + 0xBCu) - 1u);
        PE_StoreU16(voice + 0xBCu, left);
        if (left == 0u)
            PE_StoreU32(st + 0x3Cu, PE_LoadU32(st + 0x3Cu) ^ voice_mask);
    }

    if (PE_LoadU16(voice + 0x96u) != 0u) {
        PE_StoreU16(voice + 0x96u, (uint16_t)(PE_LoadU16(voice + 0x96u) - 1u));
        PE_StoreU16(voice + 0x94u,
                    (uint16_t)(PE_LoadU16(voice + 0x94u) + PE_LoadU16(voice + 0x98u)));
        value = (PE_LoadU16(voice + 0x94u) & 0x7F00u) >> 8;
        if (PE_LoadU16(voice + 0x94u) & 0x8000u)
            scaled = (value * (int)PE_LoadU32(voice + 0x30u)) >> 7;
        else
            scaled = (value * (int)(((PE_LoadU32(voice + 0x30u) << 4) -
                                     PE_LoadU32(voice + 0x30u)) >>
                                    8)) >>
                     7;
        PE_StoreU16(voice + 0x92u, (uint16_t)scaled);
        if (PE_LoadU16(voice + 0x8Au) == 0u && PE_LoadU16(voice + 0x8Eu) != 1u) {
            table = PE_LoadU32(voice + 0x1Cu);
            if (table >= 0x80000000u && table < 0x80200000u) {
                if (PE_LoadU16(table) == 0u && PE_LoadU16(table + 2u) == 0u)
                    table += (uint32_t)(int16_t)PE_LoadU16(table + 4u) * 2u;
                value = ((int16_t)PE_LoadU16(voice + 0x92u) *
                         (int16_t)PE_LoadU16(table)) >>
                        16;
                if (value != (int16_t)PE_LoadU16(voice + 0xE8u)) {
                    PE_StoreU16(voice + 0xE8u, (uint16_t)value);
                    PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 0x10u);
                    if (value >= 0)
                        PE_StoreU16(voice + 0xE8u, (uint16_t)(value << 1));
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xA8u) != 0u) {
        PE_StoreU16(voice + 0xA8u, (uint16_t)(PE_LoadU16(voice + 0xA8u) - 1u));
        PE_StoreU16(voice + 0xA6u,
                    (uint16_t)(PE_LoadU16(voice + 0xA6u) + PE_LoadU16(voice + 0xAAu)));
        if (PE_LoadU16(voice + 0x9Eu) == 0u && PE_LoadU16(voice + 0xA2u) != 1u) {
            table = PE_LoadU32(voice + 0x20u);
            if (table >= 0x80000000u && table < 0x80200000u) {
                if (PE_LoadU16(table) == 0u && PE_LoadU16(table + 2u) == 0u)
                    table += (uint32_t)(int16_t)PE_LoadU16(table + 4u) * 2u;
                value = (((int16_t)PE_LoadU16(voice + 0x46u) *
                          (PE_LoadU16(voice + 0x6Cu) >> 8)) >>
                         7) *
                        (PE_LoadU16(voice + 0xA6u) >> 8);
                value = ((value << 9) >> 16) * (int16_t)PE_LoadU16(table);
                value >>= 15;
                if (value != (int16_t)PE_LoadU16(voice + 0xEAu)) {
                    PE_StoreU16(voice + 0xEAu, (uint16_t)value);
                    PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xB6u) != 0u) {
        PE_StoreU16(voice + 0xB6u, (uint16_t)(PE_LoadU16(voice + 0xB6u) - 1u));
        PE_StoreU16(voice + 0xB4u,
                    (uint16_t)(PE_LoadU16(voice + 0xB4u) + PE_LoadU16(voice + 0xB8u)));
        if (PE_LoadU16(voice + 0xB0u) != 1u) {
            table = PE_LoadU32(voice + 0x24u);
            if (table >= 0x80000000u && table < 0x80200000u) {
                if (PE_LoadU16(table) == 0u && PE_LoadU16(table + 2u) == 0u)
                    table += (uint32_t)(int16_t)PE_LoadU16(table + 4u) * 2u;
                value = ((PE_LoadU16(voice + 0xB4u) >> 8) *
                         (int16_t)PE_LoadU16(table)) >>
                        15;
                if (value != (int16_t)PE_LoadU16(voice + 0xECu)) {
                    PE_StoreU16(voice + 0xECu, (uint16_t)value);
                    PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0x7Au) != 0u) {
        old_value = PE_LoadU32(voice + 0x34u);
        new_value = old_value + PE_LoadU32(voice + 0x4Cu);
        PE_StoreU16(voice + 0x7Au, (uint16_t)(PE_LoadU16(voice + 0x7Au) - 1u));
        if ((new_value & 0xFFFF0000u) != (old_value & 0xFFFF0000u))
            PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 0x10u);
        PE_StoreU32(voice + 0x34u, new_value);
    }

    if (PE_LoadU32(voice + 0xF4u) != f4_before)
        mark_audio_dirty(0x100u);
}

void func_80087AA8(pe_addr_t voice, uint32_t mask)
{
    update_voice_registers(voice, mask);
}

/* Stream bank update — same voice layout slides as 87AA8 until a distinct
 * Spu_TickVoiceEnvelopes donor is folded. */
void func_80087FA0(pe_addr_t voice, uint32_t mask)
{
    update_voice_registers(voice, mask);
}

/* Akao_SetVoicePitch / key-off flush — func_80089784 (khasinski misc candidate). */
void func_80089784(void)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    pe_addr_t scratch = 0x800BCD7Cu;
    uint32_t mask_out;
    uint32_t mask_keep;
    uint32_t pending_secondary;
    uint32_t pending_primary;
    uint32_t mask;
    uint32_t blocked;

    if (!st || st < 0x80000000u || st >= 0x80200000u)
        return;

    PE_StoreU32(scratch, 0);
    blocked = PE_LoadU32(0x800BCD50u) | PE_LoadU32(0x800BCD60u);
    mask_keep = ~blocked;
    pending_secondary = PE_LoadU32(st + 0x6Cu) & PE_LoadU32(st + 0x80u);
    mask = pending_secondary & PE_LoadU32(st + 0x70u);
    if (mask != 0u) {
        PE_StoreU32(0x8009D2C8u, st + 0x68u);
        func_80089724(0x800BA560u, scratch, mask, mask_keep);
        st = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, st - 0x68u);
        pending_secondary &= ~PE_LoadU32(st - 0x68u + 8u);
        PE_StoreU32(st - 0x68u + 0x18u,
                    PE_LoadU32(st - 0x68u + 0x18u) & ~PE_LoadU32(st - 0x68u + 8u));
    }

    st = PE_LoadU32(0x8009D2C8u);
    pending_primary = PE_LoadU32(st + 4u) & PE_LoadU32(st + 0x18u);
    mask = pending_primary & PE_LoadU32(st + 8u);
    if (mask != 0u) {
        func_80089724(0x800B8AC0u, scratch, mask, mask_keep);
        st = PE_LoadU32(0x8009D2C8u);
        pending_primary &= ~PE_LoadU32(st + 8u);
        PE_StoreU32(st + 0x18u, PE_LoadU32(st + 0x18u) & ~PE_LoadU32(st + 8u));
    }

    if (pending_secondary != 0u) {
        st = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, st + 0x68u);
        func_80089724(0x800BA560u, scratch, pending_secondary, mask_keep);
        PE_StoreU32(PE_LoadU32(0x8009D2C8u) + 0x18u, 0);
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }
    if (pending_primary != 0u) {
        func_80089724(0x800B8AC0u, scratch, pending_primary, mask_keep);
        PE_StoreU32(PE_LoadU32(0x8009D2C8u) + 0x18u, 0);
    }

    mask_out = PE_LoadU32(scratch) | PE_LoadU32(0x800BCD5Cu);
    PE_StoreU32(0x800BCD5Cu, 0);
    if (mask_out != 0u)
        spu_write_key_off(mask_out);
}

/* Akao_ProcessVoiceQueue — func_80089328 (khasinski candidate; 8900C/89250 stubbed). */
void func_80089328(void)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    pe_addr_t key_on_scratch = 0x800BCD74u;
    uint32_t key_on_mask;
    uint32_t blocked_mask;
    uint32_t secondary_pending, secondary_restart;
    uint32_t primary_pending, primary_restart;
    uint32_t flags;
    uint32_t bit;
    pe_addr_t voice;

    if (!st || st < 0x80000000u || st >= 0x80200000u)
        return;

    PE_StoreU32(key_on_scratch, 0);
    key_on_mask = 0;
    blocked_mask = PE_LoadU32(0x800BCD50u) | PE_LoadU32(0x800BCD60u);

    if (((PE_LoadU32(st + 4u) & PE_LoadU32(st + 0x10u)) |
         (PE_LoadU32(st + 0x6Cu) & PE_LoadU32(st + 0x78u))) != 0u)
        func_80089250(blocked_mask);

    st = PE_LoadU32(0x8009D2C8u);
    secondary_pending = (PE_LoadU32(st + 0x6Cu) & PE_LoadU32(st + 0x7Cu)) &
                        ~(PE_LoadU32(st + 0x74u) & blocked_mask);
    secondary_restart = (secondary_pending & PE_LoadU32(st + 0x74u)) & ~blocked_mask;
    if ((secondary_pending & PE_LoadU32(st + 0x70u)) != 0u) {
        PE_StoreU32(0x8009D2C8u, st + 0x68u);
        func_8008900C(0x800BA560u,
                      secondary_pending & PE_LoadU32(st + 0x70u),
                      secondary_restart, key_on_scratch);
        st = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, st - 0x68u);
        secondary_pending &= ~PE_LoadU32(st - 0x68u + 8u);
        PE_StoreU32(st - 0x68u + 0x10u,
                    PE_LoadU32(st - 0x68u + 0x10u) & ~PE_LoadU32(st - 0x68u + 8u));
    }

    st = PE_LoadU32(0x8009D2C8u);
    primary_pending = (PE_LoadU32(st + 4u) & PE_LoadU32(st + 0x14u)) &
                      ~(PE_LoadU32(st + 0xCu) & (secondary_restart | blocked_mask));
    primary_restart = (primary_pending & PE_LoadU32(st + 0xCu)) &
                      ~(secondary_restart | blocked_mask);
    if ((primary_pending & PE_LoadU32(st + 8u)) != 0u) {
        func_8008900C(0x800B8AC0u, primary_pending & PE_LoadU32(st + 8u),
                      primary_restart, key_on_scratch);
        st = PE_LoadU32(0x8009D2C8u);
        primary_pending &= ~PE_LoadU32(st + 8u);
        PE_StoreU32(st + 0x10u, PE_LoadU32(st + 0x10u) & ~PE_LoadU32(st + 8u));
    }

    if (secondary_pending != 0u) {
        st = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, st + 0x68u);
        func_8008900C(0x800BA560u, secondary_pending,
                      secondary_restart & ~primary_restart, key_on_scratch);
        PE_StoreU32(PE_LoadU32(0x8009D2C8u) + 0x10u, 0);
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }
    if (primary_pending != 0u) {
        func_8008900C(0x800B8AC0u, primary_pending, primary_restart,
                      key_on_scratch);
        PE_StoreU32(PE_LoadU32(0x8009D2C8u) + 0x10u, 0);
    }

    primary_pending = PE_LoadU32(0x800BCD50u) & PE_LoadU32(0x800BCD58u);
    if (primary_pending != 0u) {
        bit = 0x1000u;
        voice = 0x800BC000u;
        key_on_mask = PE_LoadU32(key_on_scratch) | PE_LoadU32(0x800BCD54u);
        while (primary_pending != 0u) {
            if ((primary_pending & bit) != 0u) {
                /* Key-on publish deferred to ApplyPending when F4 has 0x1000. */
                PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | 0x1000u);
                mark_audio_dirty(0x100u);
                primary_pending &= ~bit;
            }
            bit <<= 1;
            voice += 0x11Cu;
        }
        PE_StoreU32(0x800BCD54u, 0);
        PE_StoreU32(key_on_scratch, key_on_mask);
    }

    flags = PE_LoadU32(0x8009D2C4u);
    if (flags & 0x80u) {
        /* Master volume slide dirty — leave for later; clear so drain continues. */
        PE_StoreU32(0x8009D2C4u, flags & ~0x80u);
        flags &= ~0x80u;
    }
    if (flags & 0x10u)
        PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) & ~0x10u);
    /* 0x100: leave set so PE_SpuScore_ApplyDirtyVoices after the tick publishes. */

    key_on_mask = PE_LoadU32(key_on_scratch);
    if (key_on_mask != 0u)
        spu_write_key_on(key_on_mask);
}

/* Sample bytecode step — still open (needs seq opcode tables). */
void func_8008E8D0(pe_addr_t voice, uint32_t mask)
{
    (void)voice;
    (void)mask;
}

/* SPU_StepReverbLoad — still open (no donor on remote audio tip). */
void func_8008D844(void) {}

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
            mark_audio_dirty(0x80u);
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
