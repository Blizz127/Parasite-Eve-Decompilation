/*
 * Host Akao_Tick (func_8008DB7C) — score sequencer tick.
 * AUD1-E3: func_80087AA8 / func_80087FA0 are real PE-RAM ports of the
 * Spu_UpdateVoiceRegisters / Spu_TickVoiceEnvelopes slides+LFO tickers.
 * AUD1-E4: func_8008E8D0 seq bytecode ported (semantic).
 * AUD1-E5: func_80089328 Akao_ProcessVoiceQueue ported (semantic).
 * AUD1-E6: func_8008D844 (SPU_StepReverbLoad) + func_80089784
 * (Akao_SetVoicePitch / key-off flush) ported (semantic).
 * AUD1-E7: func_8008900C StepVoiceNote + func_80089250 UpdateVoiceEnvelopes
 * ported with envelope-table HW alloc + ENVX poll (semantic).
 * AUD1-E8: func_800878F0 WriteVoiceParam + func_80089F08 SpuGetVoiceEnvelope
 * — real ADSR flag publish + ENVX read (semantic).
 */
#include "pe_sdk.h"
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

static void akao_mark_audio_dirty(void)
{
    PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x100u);
}

static void akao_or_voice_pending(pe_addr_t voice, uint32_t bits)
{
    PE_StoreU32(voice + 0xF4u, PE_LoadU32(voice + 0xF4u) | bits);
    akao_mark_audio_dirty();
}

/* Spu_WriteKeyOff — host. Synth listens on 0x18A for voices 0..15; also
 * mirror retail KEY-OFF window at 0x18C/0x18E. */
static void akao_spu_write_key_off(uint32_t mask)
{
    if (!mask)
        return;
    PE_SpuRegister_StoreU16(0x18Au, (uint16_t)(mask & 0xFFFFu));
    PE_SpuRegister_StoreU16(0x18Cu, (uint16_t)(mask & 0xFFFFu));
    PE_SpuRegister_StoreU16(0x18Eu, (uint16_t)((mask >> 16) & 0xFFu));
}

/* Seq_ApplyGlobalPitch (8D7D0): publish D_8009D2B6 via SpuSetCommonAttr. */
static void akao_apply_global_pitch(void)
{
    uint16_t value = PE_LoadU16(0x8009D2B6u);
    PE_StoreU32(0x800C0D90u, 0x1C0u);
    PE_StoreU32(0x800C0DA4u, 0u);
    PE_StoreU16(0x800C0DA2u, value);
    PE_StoreU16(0x800C0DA0u, value);
    func_80085F74(0x800C0D90u);
}

/* func_8008AB9C — mark active bank voices volume-dirty. */
static void akao_mark_active_volume_dirty(pe_addr_t tracks)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    uint32_t mask;
    uint32_t bit = 1u;
    pe_addr_t voice;

    if (!st)
        return;
    mask = PE_LoadU32(st + 4u);
    voice = tracks;
    while (mask != 0u) {
        if (mask & bit) {
            akao_or_voice_pending(voice, 3u);
            mask ^= bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
    }
}

/* Host Spu_VoiceMaskCompose (89724). */
static void akao_voice_mask_compose(pe_addr_t tracks, uint32_t *mask_out,
                                   uint32_t mask, uint32_t mask_keep)
{
    uint32_t bit = 1u;
    pe_addr_t track = tracks;

    while (mask != 0u) {
        if ((mask & bit) != 0u) {
            uint32_t idx = PE_LoadU32(track + 0xF0u);
            if (idx < 24u)
                *mask_out |= 1u << idx;
        }
        mask &= ~bit;
        track += 0x11Cu;
        bit <<= 1;
    }
    *mask_out &= mask_keep;
}

/* AUD1-E6: Akao_SetVoicePitch / key-off flush (func_80089784). */
void func_80089784(void)
{
    uint32_t mask_out = 0u;
    uint32_t mask_keep;
    uint32_t pending_secondary;
    uint32_t pending_primary;
    uint32_t mask;
    pe_addr_t state;
    uint32_t blocked;

    state = PE_LoadU32(0x8009D2C8u);
    if (!state || state < 0x80000000u || state >= 0x80200000u)
        return;

    blocked = PE_LoadU32(0x800BCD50u) | PE_LoadU32(0x800BCD60u);
    mask_keep = ~blocked;
    pending_secondary = PE_LoadU32(state + 0x6Cu) & PE_LoadU32(state + 0x80u);
    mask = pending_secondary & PE_LoadU32(state + 0x70u);
    if (mask != 0u) {
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        akao_voice_mask_compose(0x800BA560u, &mask_out, mask, mask_keep);
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, state - 0x68u);
        pending_secondary &= ~PE_LoadU32(state + 8u);
        PE_StoreU32(state + 0x18u,
                    PE_LoadU32(state + 0x18u) & ~PE_LoadU32(state + 8u));
    }

    state = PE_LoadU32(0x8009D2C8u);
    pending_primary = PE_LoadU32(state + 4u) & PE_LoadU32(state + 0x18u);
    mask = pending_primary & PE_LoadU32(state + 8u);
    if (mask != 0u) {
        akao_voice_mask_compose(0x800B8AC0u, &mask_out, mask, mask_keep);
        state = PE_LoadU32(0x8009D2C8u);
        pending_primary &= ~PE_LoadU32(state + 8u);
        PE_StoreU32(state + 0x18u,
                    PE_LoadU32(state + 0x18u) & ~PE_LoadU32(state + 8u));
    }

    if (pending_secondary != 0u) {
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        akao_voice_mask_compose(0x800BA560u, &mask_out, pending_secondary,
                                mask_keep);
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(state + 0x18u, 0u);
        PE_StoreU32(0x8009D2C8u, state - 0x68u);
    }

    if (pending_primary != 0u) {
        akao_voice_mask_compose(0x800B8AC0u, &mask_out, pending_primary,
                                mask_keep);
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(state + 0x18u, 0u);
    }

    mask_out |= PE_LoadU32(0x800BCD5Cu);
    PE_StoreU32(0x800BCD5Cu, 0u);
    if (mask_out != 0u)
        akao_spu_write_key_off(mask_out);
}

/* AUD1-E6: SPU_StepReverbLoad (func_8008D844). */
void func_8008D844(void)
{
    uint16_t tick;
    pe_addr_t state;
    uint32_t old_value;
    uint32_t new_value;
    uint32_t mask;
    uint32_t bit;
    pe_addr_t voice;
    unsigned i;
    uint16_t duration;

    tick = (uint16_t)(PE_LoadU16(0x8009CDECu) + 1u);
    PE_StoreU16(0x8009CDECu, tick);
    if ((tick & 3u) != 0u)
        return;

    if ((int16_t)PE_LoadU16(0x8009D2A2u) != 0) {
        PE_StoreU16(0x8009D2A2u,
                    (uint16_t)((int16_t)PE_LoadU16(0x8009D2A2u) - 1));
        PE_StoreU32(0x8009D2B4u,
                    PE_LoadU32(0x8009D2B4u) + PE_LoadU32(0x8009D284u));
        akao_apply_global_pitch();
    }

    if ((int16_t)PE_LoadU16(0x8009D220u) != 0) {
        PE_StoreU16(0x8009D220u,
                    (uint16_t)((int16_t)PE_LoadU16(0x8009D220u) - 1));
        PE_StoreU32(0x8009D2D0u,
                    PE_LoadU32(0x8009D2D0u) + PE_LoadU32(0x8009D214u));
    }

    if ((int16_t)PE_LoadU16(0x8009D21Eu) != 0) {
        PE_StoreU16(0x8009D21Eu,
                    (uint16_t)((int16_t)PE_LoadU16(0x8009D21Eu) - 1));
        old_value = PE_LoadU32(0x8009D2CCu);
        new_value = old_value + PE_LoadU32(0x8009D210u);
        if ((new_value & 0x00FF0000u) != (old_value & 0x00FF0000u)) {
            for (i = 0u; i < 24u; i++)
                akao_or_voice_pending(0x800B8AC0u + i * 0x11Cu, 0x10u);
        }
        PE_StoreU32(0x8009D2CCu, new_value);
    }

    state = PE_LoadU32(0x8009D2C8u);
    if (state && state >= 0x80000000u && state < 0x80200000u &&
        PE_LoadU32(state + 4u) != 0u) {
        duration = PE_LoadU16(state + 0x50u);
        if (duration != 0u) {
            old_value = PE_LoadU32(state + 0x48u);
            new_value = old_value + PE_LoadU32(state + 0x4Cu);
            PE_StoreU16(state + 0x50u, (uint16_t)(duration - 1u));
            if ((new_value & 0x007F0000u) != (old_value & 0x007F0000u))
                akao_mark_active_volume_dirty(0x800B8AC0u);
            state = PE_LoadU32(0x8009D2C8u);
            PE_StoreU32(state + 0x48u, new_value);
        }
    }

    state = PE_LoadU32(0x8009D2C8u);
    if (state && state >= 0x80000000u && state < 0x80200000u) {
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        if (PE_LoadU32(state + 0x6Cu) != 0u) {
            duration = PE_LoadU16(state + 0xB8u);
            if (duration != 0u) {
                old_value = PE_LoadU32(state + 0xB0u);
                new_value = old_value + PE_LoadU32(state + 0xB4u);
                PE_StoreU16(state + 0xB8u, (uint16_t)(duration - 1u));
                if ((new_value & 0x007F0000u) != (old_value & 0x007F0000u))
                    akao_mark_active_volume_dirty(0x800BA560u);
                state = PE_LoadU32(0x8009D2C8u);
                PE_StoreU32(state + 0x48u, new_value);
            }
        }
        PE_StoreU32(0x8009D2C8u, PE_LoadU32(0x8009D2C8u) - 0x68u);
    }

    mask = PE_LoadU32(0x800BCD50u);
    bit = 0x1000u;
    voice = 0x800BC03Cu;
    while (mask != 0u) {
        if ((mask & bit) != 0u) {
            duration = PE_LoadU16(voice + 0x38u);
            if (duration != 0u) {
                int16_t cur = (int16_t)PE_LoadU16(voice + 0x9Cu);
                int16_t delta = (int16_t)PE_LoadU16(voice + 0x9Eu);
                int16_t sum;
                PE_StoreU16(voice + 0x38u, (uint16_t)(duration - 1u));
                sum = (int16_t)(cur + delta);
                if (((uint16_t)sum & 0xFF00u) != ((uint16_t)cur & 0xFF00u))
                    akao_or_voice_pending(voice - 0x3Cu, 3u);
                PE_StoreU16(voice + 0x9Cu, (uint16_t)sum);
            }

            duration = PE_LoadU16(voice + 0x3Cu);
            if (duration != 0u) {
                uint16_t cur = PE_LoadU16(voice + 0x3Au);
                int16_t delta = (int16_t)PE_LoadU16(voice + 0xA0u);
                uint16_t sum;
                PE_StoreU16(voice + 0x3Cu, (uint16_t)(duration - 1u));
                sum = (uint16_t)(cur + (uint16_t)delta);
                if ((sum & 0xFF00u) != (cur & 0xFF00u))
                    akao_or_voice_pending(voice - 0x3Cu, 3u);
                PE_StoreU16(voice + 0x3Au, sum);
            }

            duration = PE_LoadU16(voice + 0x34u);
            if (duration != 0u) {
                old_value = PE_LoadU32(voice + 0x00u);
                new_value = old_value + PE_LoadU32(voice + 0x04u);
                PE_StoreU16(voice + 0x34u, (uint16_t)(duration - 1u));
                if ((new_value & 0xFF00u) != (old_value & 0xFF00u))
                    akao_or_voice_pending(voice - 0x3Cu, 0x10u);
                PE_StoreU32(voice + 0x00u, new_value);
            }

            mask ^= bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
    }
}

/* ---- AUD1-E5: Akao_ProcessVoiceQueue (func_80089328) ---- */

static void akao_spu_or_key_on(uint32_t mask)
{
    if (!mask)
        return;
    PE_SpuRegister_StoreU16(0x188u,
        (uint16_t)(PE_SpuRegister_LoadU16(0x188u) | (mask & 0xFFFFu)));
    PE_SpuRegister_StoreU16(0x18Au,
        (uint16_t)(PE_SpuRegister_LoadU16(0x18Au) | ((mask >> 16) & 0xFFu)));
}

/* Host SpuGetVoiceEnvelope (89F08): read SPU ENVX (voice*0x10 + 0xC). */
static void akao_spu_get_voice_envelope(uint32_t hw_index, pe_addr_t out_s16)
{
    uint16_t envx = PE_SpuRegister_LoadU16(hw_index * 0x10u + 0xCu);
    PE_StoreU16(out_s16, envx);
}

/* Akao_RemoveVoice (89218): clear matching +0xF0 slots back to 24. */
static void akao_remove_voice(pe_addr_t voices, uint32_t hw_index)
{
    uint32_t i;
    pe_addr_t slot = voices + 0xF0u;
    for (i = 0u; i < 24u; i++) {
        if (PE_LoadU32(slot) == hw_index)
            PE_StoreU32(slot, 24u);
        slot += 0x11Cu;
    }
}

/* Host ADSR bitfield helpers (AkaoSpuVoice_SetAdsr*). */
static void akao_spu_set_adsr_attack(uint32_t hw, uint32_t rate, uint32_t mode)
{
    uint32_t reg = hw * 0x10u + 8u;
    uint16_t cur = PE_SpuRegister_LoadU16(reg);
    uint16_t value = (uint16_t)((((mode >> 2) << 15) | (rate << 8)) | (cur & 0xFFu));
    PE_SpuRegister_StoreU16(reg, value);
}
static void akao_spu_set_adsr_decay(uint32_t hw, uint32_t rate)
{
    uint32_t reg = hw * 0x10u + 8u;
    uint16_t cur = PE_SpuRegister_LoadU16(reg);
    PE_SpuRegister_StoreU16(reg, (uint16_t)((cur & 0xFF0Fu) | (rate << 4)));
}
static void akao_spu_set_adsr_sustain_level(uint32_t hw, uint32_t level)
{
    uint32_t reg = hw * 0x10u + 8u;
    uint16_t cur = PE_SpuRegister_LoadU16(reg);
    PE_SpuRegister_StoreU16(reg, (uint16_t)((cur & 0xFFF0u) | (level & 0xFu)));
}
static void akao_spu_set_adsr_sustain_rate(uint32_t hw, uint32_t rate, uint32_t mode)
{
    uint32_t reg = hw * 0x10u + 0xAu;
    uint16_t cur = PE_SpuRegister_LoadU16(reg);
    uint16_t value = (uint16_t)((cur & 0x3Fu) |
        (((mode >> 1) << 14) | (rate << 6)));
    PE_SpuRegister_StoreU16(reg, value);
}
static void akao_spu_set_adsr_release(uint32_t hw, uint32_t rate, uint32_t mode)
{
    uint32_t reg = hw * 0x10u + 0xAu;
    uint16_t cur = PE_SpuRegister_LoadU16(reg);
    uint16_t value = (uint16_t)((cur & 0xFFC0u) |
        (((mode >> 2) << 5) | (rate & 0x1Fu)));
    PE_SpuRegister_StoreU16(reg, value);
}

/* Host Akao_WriteVoiceParam (878F0): retail AKAO_VOICE_PARAM_* flags.
 * `voice` is the full track base; params overlay begins at +0xF0. */
static void akao_write_voice_param(uint32_t hw_index, pe_addr_t voice)
{
    uint32_t flags;
    uint32_t cur;
    uint32_t reg;
    pe_addr_t p;

    if (hw_index >= 24u)
        return;
    flags = PE_LoadU32(voice + 0xF4u);
    if (!flags)
        return;
    reg = hw_index * 0x10u;
    p = voice + 0xF0u; /* AkaoVoiceParams overlay */

    if (flags & 0x10u) {
        /* Prefer computed pitch at +0x10C; fall back to slide/LFO. */
        uint16_t out = PE_LoadU16(voice + 0x10Cu);
        if (out == 0u) {
            uint32_t base = PE_LoadU32(voice + 0x30u);
            uint32_t slide = PE_LoadU32(voice + 0x34u);
            int32_t lfo = (int16_t)PE_LoadU16(voice + 0xE8u);
            if ((base & 0xFFFF0000u) != 0u || slide != 0u)
                out = (uint16_t)((int32_t)((base + slide) >> 16) + lfo);
            else
                out = (uint16_t)((int32_t)(base & 0xFFFFu) + lfo);
        }
        PE_SpuRegister_StoreU16(reg + 4u, out);
        cur = PE_LoadU32(voice + 0xF4u) & ~0x10u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x3u) {
        /* Prefer prepared +0x118/+0x11A; else raw +0x76/+0x78. */
        uint16_t left = PE_LoadU16(voice + 0x118u);
        uint16_t right = PE_LoadU16(voice + 0x11Au);
        if (left == 0u && right == 0u) {
            left = PE_LoadU16(voice + 0x76u);
            right = PE_LoadU16(voice + 0x78u);
        }
        func_80087798(hw_index, left, right);
        cur = PE_LoadU32(voice + 0xF4u) & ~0x3u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x80u) {
        PE_SpuRegister_StoreU16(reg + 6u,
                                (uint16_t)(PE_LoadU32(p + 8u) >> 3));
        cur = PE_LoadU32(voice + 0xF4u) & ~0x80u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x10000u) {
        PE_SpuRegister_StoreU16(reg + 0xEu,
                                (uint16_t)(PE_LoadU32(p + 0xCu) >> 3));
        cur = PE_LoadU32(voice + 0xF4u) & ~0x10000u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x2200u) {
        akao_spu_set_adsr_sustain_rate(hw_index, PE_LoadU16(p + 0x24u),
                                       PE_LoadU32(p + 0x14u));
        cur = PE_LoadU32(voice + 0xF4u) & ~0x2200u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x900u) {
        akao_spu_set_adsr_attack(hw_index, PE_LoadU16(p + 0x1Eu),
                                 PE_LoadU32(p + 0x10u));
        cur = PE_LoadU32(voice + 0xF4u) & ~0x900u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x4400u) {
        akao_spu_set_adsr_release(hw_index, PE_LoadU16(p + 0x26u),
                                  PE_LoadU32(p + 0x18u));
        cur = PE_LoadU32(voice + 0xF4u) & ~0x4400u;
        PE_StoreU32(voice + 0xF4u, cur);
        if (cur == 0u)
            return;
    }
    if (flags & 0x9000u) {
        akao_spu_set_adsr_decay(hw_index, PE_LoadU16(p + 0x20u));
        akao_spu_set_adsr_sustain_level(hw_index, PE_LoadU16(p + 0x22u));
    }
    /* Retail zeroes remaining pending; key-on is via ProcessVoiceQueue. */
    PE_StoreU32(voice + 0xF4u, 0u);
    (void)p;
}

/* Akao_StepVoiceNote (8900C): envelope-table HW alloc + key-on accumulate. */
static void akao_step_voice_note(pe_addr_t voices, uint32_t active_mask,
                                 uint32_t restart_mask, uint32_t *key_on_mask)
{
    uint32_t bit = 1u;
    uint32_t track = 0u;
    pe_addr_t voice = voices;
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    uint32_t gated = st ? (active_mask & PE_LoadU32(st + 0x10u)) : 0u;
    const uint32_t pending_cluster = 0x1FF93u;

    while (active_mask != 0u) {
        if ((active_mask & bit) != 0u) {
            uint32_t pending = PE_LoadU32(voice + 0xF4u);
            if (pending != 0u) {
                if ((gated & bit) != 0u) {
                    if ((restart_mask & bit) != 0u) {
                        *key_on_mask |= 1u << track;
                        PE_StoreU32(voice + 0xF0u, track);
                        akao_or_voice_pending(voice, pending_cluster);
                    } else {
                        uint32_t hw;
                        for (hw = 0u; hw < 24u; hw++) {
                            pe_addr_t env = 0x800B002Cu + hw * 8u;
                            if ((int16_t)PE_LoadU16(env) == 0) {
                                akao_or_voice_pending(voice, pending_cluster);
                                *key_on_mask |= 1u << hw;
                                PE_StoreU32(voice + 0xF0u, hw);
                                PE_StoreU16(env, 0x7FFFu);
                                akao_mark_audio_dirty();
                                break;
                            }
                        }
                        if (hw == 24u) {
                            PE_StoreU32(voice + 0xF0u, 24u);
                            if (st)
                                PE_StoreU32(st, PE_LoadU32(st) | 1u);
                        }
                    }
                }

                if ((PE_LoadU32(0x8009D2B8u) & bit) != 0u) {
                    PE_StoreU16(voice + 0x11Au, 0);
                    PE_StoreU16(voice + 0x118u, 0);
                }

                {
                    uint32_t hw = PE_LoadU32(voice + 0xF0u);
                    if (hw < 24u)
                        akao_write_voice_param(hw, voice);
                }
            }
            active_mask &= ~bit;
        }
        voice += 0x11Cu;
        bit <<= 1;
        track++;
    }
}

/* Akao_UpdateVoiceEnvelopes (89250): ENVX poll + RemoveVoice on decay. */
static void akao_update_voice_envelopes(uint32_t blocked_mask)
{
    pe_addr_t st = PE_LoadU32(0x8009D2C8u);
    uint32_t protected_mask;
    uint32_t voice_index;

    if (!st || st < 0x80000000u || st >= 0x80200000u)
        return;

    protected_mask =
        ((PE_LoadU32(st + 4u) & PE_LoadU32(st + 0xCu)) |
         (PE_LoadU32(st + 0x6Cu) & PE_LoadU32(st + 0x74u))) |
        blocked_mask;

    for (voice_index = 0u; voice_index < 24u; voice_index++) {
        pe_addr_t env = 0x800B002Cu + voice_index * 8u;
        if ((protected_mask & (1u << voice_index)) != 0u) {
            PE_StoreU16(env, 0x7FFFu);
        } else {
            akao_spu_get_voice_envelope(voice_index, env);
            if ((int16_t)PE_LoadU16(env) == 0) {
                akao_remove_voice(0x800B8AC0u, voice_index);
                akao_remove_voice(0x800BA560u, voice_index);
            }
        }
    }
}

static void akao_set_voice_key_on_stream(pe_addr_t voice, uint32_t voice_mask)
{
    (void)voice_mask;
    akao_or_voice_pending(voice, 0x1000u | 0x13u);
}

void func_80089328(void)
{
    uint32_t key_on_mask = 0u;
    uint32_t blocked_mask;
    uint32_t secondary_pending;
    uint32_t secondary_restart;
    uint32_t primary_pending;
    uint32_t primary_restart;
    uint32_t flags;
    pe_addr_t state;
    uint32_t bit;
    pe_addr_t voice;
    pe_addr_t voice_tail;

    blocked_mask = PE_LoadU32(0x800BCD50u) | PE_LoadU32(0x800BCD60u);

    state = PE_LoadU32(0x8009D2C8u);
    if (!state || state < 0x80000000u || state >= 0x80200000u)
        return;

    if (((PE_LoadU32(state + 4u) & PE_LoadU32(state + 0x10u)) |
         (PE_LoadU32(state + 0x6Cu) & PE_LoadU32(state + 0x78u))) != 0u)
        akao_update_voice_envelopes(blocked_mask);

    state = PE_LoadU32(0x8009D2C8u);
    secondary_pending =
        (PE_LoadU32(state + 0x6Cu) & PE_LoadU32(state + 0x7Cu)) &
        ~(PE_LoadU32(state + 0x74u) & blocked_mask);
    secondary_restart =
        (secondary_pending & PE_LoadU32(state + 0x74u)) & ~blocked_mask;
    if ((secondary_pending & PE_LoadU32(state + 0x70u)) != 0u) {
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        akao_step_voice_note(0x800BA560u,
                             secondary_pending & PE_LoadU32(state + 0x70u),
                             secondary_restart, &key_on_mask);

        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, state - 0x68u);
        secondary_pending &= ~PE_LoadU32(state + 8u);
        PE_StoreU32(state + 0x10u,
                    PE_LoadU32(state + 0x10u) & ~PE_LoadU32(state + 8u));
    }

    state = PE_LoadU32(0x8009D2C8u);
    primary_pending =
        (PE_LoadU32(state + 4u) & PE_LoadU32(state + 0x14u)) &
        ~(PE_LoadU32(state + 0xCu) & (secondary_restart | blocked_mask));
    primary_restart =
        (primary_pending & PE_LoadU32(state + 0xCu)) &
        ~(secondary_restart | blocked_mask);
    if ((primary_pending & PE_LoadU32(state + 8u)) != 0u) {
        akao_step_voice_note(0x800B8AC0u,
                             primary_pending & PE_LoadU32(state + 8u),
                             primary_restart, &key_on_mask);

        state = PE_LoadU32(0x8009D2C8u);
        primary_pending &= ~PE_LoadU32(state + 8u);
        PE_StoreU32(state + 0x10u,
                    PE_LoadU32(state + 0x10u) & ~PE_LoadU32(state + 8u));
    }

    if (secondary_pending != 0u) {
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        akao_step_voice_note(0x800BA560u, secondary_pending,
                             secondary_restart & ~primary_restart,
                             &key_on_mask);
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(state + 0x10u, 0u);
        PE_StoreU32(0x8009D2C8u, state - 0x68u);
    }

    if (primary_pending != 0u) {
        akao_step_voice_note(0x800B8AC0u, primary_pending, primary_restart,
                             &key_on_mask);
        state = PE_LoadU32(0x8009D2C8u);
        PE_StoreU32(state + 0x10u, 0u);
    }

    primary_pending = PE_LoadU32(0x800BCD50u) & PE_LoadU32(0x800BCD58u);
    if (primary_pending != 0u) {
        bit = 0x1000u;
        voice = 0x800BC000u;
        voice_tail = voice + 0x38u;
        key_on_mask |= PE_LoadU32(0x800BCD54u);

        while (primary_pending != 0u) {
            if ((primary_pending & bit) != 0u) {
                akao_set_voice_key_on_stream(voice, bit);
                if (PE_LoadU32(voice_tail + 0xBCu) != 0u)
                    akao_or_voice_pending(voice, PE_LoadU32(voice_tail + 0xBCu));
                primary_pending &= ~bit;
            }
            bit <<= 1;
            voice_tail += 0x11Cu;
            voice += 0x11Cu;
        }

        PE_StoreU32(0x800BCD54u, 0u);
    }

    flags = PE_LoadU32(0x8009D2C4u);
    if ((flags & 0x80u) != 0u) {
        /* Master volume slide consume — leave for ApplyDirty / common attr. */
        PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) & ~0x80u);
    }

    if ((flags & 0x10u) != 0u) {
        uint16_t noise_clock;
        if (PE_LoadU32(0x800BCD50u) != 0u)
            noise_clock = PE_LoadU16(0x800BCD78u);
        else
            noise_clock = PE_LoadU16(PE_LoadU32(0x8009D2C8u) + 0x5Au);
        {
            uint16_t cnt = PE_SpuRegister_LoadU16(0x1AAu);
            cnt = (uint16_t)((cnt & ~0x3F00u) | ((noise_clock & 0x3Fu) << 8));
            PE_SpuRegister_StoreU16(0x1AAu, cnt);
        }
        PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) & ~0x10u);
    }

    /* Host adaptation: retail 0x100 flush writes ADSR/vol/addr via 89B48/…
     * Keep bit set so PE_SpuScore_ApplyDirtyVoices (after Akao_Tick) publishes
     * voice +0xF4 pending. Also push reverb/noise/FM enables. */
    if ((flags & 0x100u) != 0u) {
        uint32_t rev = PE_LoadU32(0x800C0DD0u);
        uint32_t noise = PE_LoadU32(0x800C0DD4u);
        uint32_t fm = PE_LoadU32(0x800C0DD8u);
        PE_SpuRegister_StoreU16(0x198u, (uint16_t)rev);
        PE_SpuRegister_StoreU16(0x19Au, (uint16_t)(rev >> 16));
        PE_SpuRegister_StoreU16(0x194u, (uint16_t)noise);
        PE_SpuRegister_StoreU16(0x196u, (uint16_t)(noise >> 16));
        PE_SpuRegister_StoreU16(0x190u, (uint16_t)fm);
        PE_SpuRegister_StoreU16(0x192u, (uint16_t)(fm >> 16));
        /* intentionally do not clear 0x100 */
    }

    if (key_on_mask != 0u)
        akao_spu_or_key_on(key_on_mask);
}

/* ---- AUD1-E4: seq bytecode when voice +0x56 hits 0 (func_8008E8D0) ---- */

/* Note duration table at D_8009B8DC (retail). */
static const uint16_t akao_note_lengths[12] = {
    192, 96, 48, 24, 12, 6, 3, 32, 16, 8, 4, 0
};

static uint32_t akao_opcode_mod14(uint32_t op)
{
    return op - (op / 14u) * 14u;
}

static uint32_t akao_lookup_pitch(uint32_t note, uint32_t key, int detune)
{
    pe_addr_t table;
    uint32_t pitch;
    (void)detune;
    if (note >= 0x80u)
        note = 0;
    table = 0x800B2900u + (note << 6);
    /* instrument pitch[0] at +0x10; scale by key nibble coarsely */
    pitch = PE_LoadU32(table + 0x10u);
    if (key) {
        uint32_t k = key & 0x7Fu;
        if (k > 1u)
            pitch = (pitch * k) >> 6;
    }
    return pitch ? pitch : 0x1000u;
}

static void akao_set_voice_instrument(pe_addr_t voice, pe_addr_t instrument,
                                      uint32_t sample_header)
{
    uint32_t flags = PE_LoadU32(voice + 0x38u);
    PE_StoreU32(voice + 0xF8u, sample_header);
    PE_StoreU32(voice + 0xFCu, PE_LoadU32(instrument + 4u));
    PE_StoreU16(voice + 0x10Eu, PE_LoadU8(instrument + 8u));
    PE_StoreU16(voice + 0x110u, PE_LoadU8(instrument + 9u));
    PE_StoreU16(voice + 0x112u, PE_LoadU8(instrument + 10u));
    PE_StoreU16(voice + 0x114u, PE_LoadU8(instrument + 11u));
    PE_StoreU32(voice + 0x100u, PE_LoadU8(instrument + 13u));
    PE_StoreU32(voice + 0x104u, PE_LoadU8(instrument + 14u));
    if (flags & 0x200u) {
        akao_or_voice_pending(voice, 0x0001BB80u);
    } else {
        PE_StoreU16(voice + 0x116u, PE_LoadU8(instrument + 12u));
        PE_StoreU32(voice + 0x108u, PE_LoadU8(instrument + 15u));
        akao_or_voice_pending(voice, 0x0001FF80u);
    }
}

/* Operand sizes for common high opcodes (best-effort; unknown → 0 extra). */
static uint32_t akao_high_op_extra(uint32_t op)
{
    switch (op) {
    case 0xA5: case 0xA6: case 0xA7: /* panpot step leaves */
    case 0xAC: case 0xAD: case 0xCC: case 0xCD:
    case 0xD0: case 0xD1: case 0xDB:
        return 0;
    case 0xA1: case 0xA3: case 0xA8: case 0xA9:
    case 0xAA: case 0xAB: case 0xAE: case 0xAF:
    case 0xB1: case 0xB2: case 0xC0: case 0xC1:
        return 1;
    case 0xA2: case 0xA4: case 0xB0: case 0xB4:
    case 0xC2: case 0xC3: case 0xC8: case 0xC9:
        return 2;
    case 0xB5: case 0xB6: case 0xB8: case 0xB9:
    case 0xBA: case 0xBC: case 0xBD: case 0xBE:
        return 3;
    default:
        return 1; /* conservative consume */
    }
}

static void akao_dispatch_high(pe_addr_t voice, uint32_t mask, uint32_t op)
{
    pe_addr_t pc = PE_LoadU32(voice);
    uint32_t extra = akao_high_op_extra(op);
    (void)mask;
    /* Prefer already-ported tiny leaves when opcode maps to them. */
    switch (op) {
    case 0xA5: /* sndTrackReadPanpot-ish @ 8F84C */
        if (pc) {
            PE_StoreU16(voice + 0x7Cu, PE_LoadU8(pc));
            PE_StoreU32(voice, pc + 1u);
        }
        return;
    case 0xA6: /* panpot ++ */
        PE_StoreU16(voice + 0x7Cu, (uint16_t)((PE_LoadU16(voice + 0x7Cu) + 1u) & 0xFu));
        return;
    case 0xA7: /* panpot -- */
        PE_StoreU16(voice + 0x7Cu, (uint16_t)((PE_LoadU16(voice + 0x7Cu) - 1u) & 0xFu));
        return;
    case 0xCC: /* set field 0x84 = 1 @ 904A0 */
        PE_StoreU16(voice + 0x84u, 1u);
        return;
    case 0xDB: /* clear +0x82 @ 8FCB4 */
        PE_StoreU16(voice + 0x82u, 0u);
        return;
    default:
        break;
    }
    if (pc && extra)
        PE_StoreU32(voice, pc + extra);
}

void func_8008E8D0(pe_addr_t voice, uint32_t mask)
{
    uint32_t loops = 0;
    uint32_t opcode = 0;
    uint32_t art;
    uint16_t gate;

    for (;;) {
        pe_addr_t pc = PE_LoadU32(voice);
        if (!pc)
            return;
        opcode = PE_LoadU8(pc);
        PE_StoreU32(voice, pc + 1u);

        if (opcode < 0xA0u)
            break;

        if (opcode == 0xFCu) {
            uint32_t sub = PE_LoadU8(pc + 1u);
            PE_StoreU32(voice, pc + 2u);
            /* FC sub-ops: treat unknown as no-op (handlers live in asm). */
            (void)sub;
        } else if (opcode == 0xCAu) {
            if (PE_LoadU32(voice + 0x38u) & 0x200000u) {
                PE_StoreU32(0x800BCD5Cu, PE_LoadU32(0x800BCD5Cu) | mask);
                opcode = 0xA0u;
            }
            akao_dispatch_high(voice, mask, opcode);
        } else {
            akao_dispatch_high(voice, mask, opcode);
        }

        if (opcode < 0xA0u || opcode == 0xA0u)
            break;
        if (++loops > 0x100u)
            return;
    }

    if (opcode == 0xA0u) {
        if (PE_LoadU16(voice + 0x54u) == 0u) {
            pe_addr_t st = PE_LoadU32(0x8009D2C8u);
            if ((PE_LoadU32(st + 0x14u) & mask) &&
                PE_LoadU32(voice + 0xF0u) < 24u)
                PE_StoreU32(st + 0x18u, PE_LoadU32(st + 0x18u) | mask);
        }
        return;
    }

    /* Articulation / length class from bank byte (LookupSampleBankByte). */
    {
        pe_addr_t prog = PE_LoadU32(voice + 0x14u);
        art = prog ? PE_LoadU8(prog) : 0x80u;
    }

    {
        int16_t leg = (int16_t)PE_LoadU16(voice + 0xD2u);
        if (leg) {
            PE_StoreU16(voice + 0x56u, (uint16_t)leg);
            PE_StoreU16(voice + 0x58u, (uint16_t)leg);
        }
    }

    gate = PE_LoadU16(voice + 0x56u);
    if (gate == 0u) {
        uint32_t len = akao_note_lengths[akao_opcode_mod14(opcode) % 12u];
        gate = (uint16_t)len;
        PE_StoreU16(voice + 0x56u, gate);
        if (art < 0x8Fu && art < 0x84u &&
            (PE_LoadU16(voice + 0x84u) & 5u) == 0u && gate >= 2u)
            PE_StoreU16(voice + 0x58u, (uint16_t)(gate - 2u));
        else if (art >= 0x8Fu && (PE_LoadU16(voice + 0x84u) & 5u) == 0u &&
                 gate >= 2u)
            PE_StoreU16(voice + 0x58u, (uint16_t)(gate - 2u));
        else
            PE_StoreU16(voice + 0x58u, gate);
    } else if (art < 0x8Fu && art < 0x84u &&
               (PE_LoadU16(voice + 0x84u) & 5u) == 0u) {
        uint16_t k = PE_LoadU16(voice + 0x58u);
        if (k >= 2u)
            PE_StoreU16(voice + 0x58u, (uint16_t)(k - 2u));
    }

    akao_or_voice_pending(voice, 0x4000u);
    PE_StoreU16(voice + 0xD0u, PE_LoadU16(voice + 0x56u));

    if (art < 0x8Fu)
        PE_StoreU32(voice + 0x38u, PE_LoadU32(voice + 0x38u) & ~0x40u);
    else
        PE_StoreU32(voice + 0x38u, PE_LoadU32(voice + 0x38u) | 0x40u);

    /* Rest-class 0x84..0x8E */
    if (opcode < 0x8Fu && opcode >= 0x84u) {
        if (PE_LoadU16(voice + 0x54u) == 0u) {
            pe_addr_t st = PE_LoadU32(0x8009D2C8u);
            if ((PE_LoadU32(st + 0x14u) & mask) &&
                PE_LoadU32(voice + 0xF0u) < 24u)
                PE_StoreU32(st + 0x18u, PE_LoadU32(st + 0x18u) | mask);
        }
        PE_StoreU16(voice + 0x82u, 0);
        PE_StoreU16(voice + 0xE8u, 0);
        PE_StoreU16(voice + 0xEAu, 0);
        PE_StoreU16(voice + 0x84u, (uint16_t)(PE_LoadU16(voice + 0x84u) & 0xFFFDu));
        return;
    }

    /* Note-on: select instrument by current note_pitch or opcode class */
    {
        uint32_t note = PE_LoadU16(voice + 0x5Au);
        uint32_t nmod = akao_opcode_mod14(opcode);
        uint32_t key = nmod + (uint32_t)PE_LoadU16(voice + 0x7Cu) * 6u;
        pe_addr_t instrument;
        uint32_t pitch;

        if (note == 0xFFu || note == 0u)
            note = nmod;
        if (note >= 0x80u)
            note &= 0x7Fu;
        instrument = 0x800B2900u + (note << 6);
        akao_set_voice_instrument(voice, instrument, PE_LoadU32(instrument));
        PE_StoreU16(voice + 0x5Au, (uint16_t)note);

        pitch = akao_lookup_pitch(note, key, (int16_t)PE_LoadU16(voice + 0xE0u));
        PE_StoreU32(voice + 0x30u, pitch);
        akao_or_voice_pending(voice, 0x13u);

        if (PE_LoadU16(voice + 0x54u) == 0u) {
            pe_addr_t st = PE_LoadU32(0x8009D2C8u);
            PE_StoreU32(st + 0x10u, PE_LoadU32(st + 0x10u) | mask);
            PE_StoreU32(st + 0x14u, PE_LoadU32(st + 0x14u) | mask);
        } else {
            PE_StoreU32(0x800BCD54u, PE_LoadU32(0x800BCD54u) | mask);
            PE_StoreU32(0x800BCD58u, PE_LoadU32(0x800BCD58u) | mask);
        }
    }
}



static pe_addr_t akao_wave_table(pe_addr_t table)
{
    if (!table)
        return 0;
    if (PE_LoadU16(table) == 0u && PE_LoadU16(table + 2u) == 0u)
        return table + (uint32_t)(int16_t)PE_LoadU16(table + 4u) * 2u;
    return table;
}

/* Spu_UpdateVoiceRegisters — primary/secondary bank voice ticker. */
void func_80087AA8(pe_addr_t voice, uint32_t mask)
{
    uint32_t old_value, new_value;
    int value, scaled;
    pe_addr_t table;

    if (PE_LoadU16(voice + 0x72u)) {
        old_value = PE_LoadU32(voice + 0x44u);
        new_value = old_value + PE_LoadU32(voice + 0x48u);
        PE_StoreU16(voice + 0x72u, (uint16_t)(PE_LoadU16(voice + 0x72u) - 1u));
        if ((new_value & 0xFFE00000u) != (old_value & 0xFFE00000u))
            akao_or_voice_pending(voice, 3u);
        PE_StoreU32(voice + 0x44u, new_value);
    }

    if (PE_LoadU16(voice + 0x60u)) {
        PE_StoreU16(voice + 0x60u, (uint16_t)(PE_LoadU16(voice + 0x60u) - 1u));
        PE_StoreU16(voice + 0x5Eu,
                    (uint16_t)(PE_LoadU16(voice + 0x5Eu) + PE_LoadU16(voice + 0xD6u)));
        akao_or_voice_pending(voice, 3u);
    }

    if (PE_LoadU16(voice + 0x6Eu)) {
        old_value = PE_LoadU16(voice + 0x6Cu);
        new_value = old_value + (uint32_t)(int16_t)PE_LoadU16(voice + 0xD4u);
        PE_StoreU16(voice + 0x6Eu, (uint16_t)(PE_LoadU16(voice + 0x6Eu) - 1u));
        if ((new_value & 0x7F00u) != (old_value & 0x7F00u))
            akao_or_voice_pending(voice, 3u);
        PE_StoreU16(voice + 0x6Cu, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x74u)) {
        old_value = (uint32_t)(int16_t)PE_LoadU16(voice + 0xD8u);
        new_value = old_value + (uint32_t)(int16_t)PE_LoadU16(voice + 0xDAu);
        PE_StoreU16(voice + 0x74u, (uint16_t)(PE_LoadU16(voice + 0x74u) - 1u));
        if ((PE_LoadU32(voice + 0x38u) & 0x100u) &&
            ((new_value & 0xFF00u) != (old_value & 0xFF00u)))
            akao_or_voice_pending(voice, 3u);
        PE_StoreU16(voice + 0xD8u, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x78u)) {
        old_value = PE_LoadU16(voice + 0x76u);
        new_value = old_value + (uint32_t)(int16_t)PE_LoadU16(voice + 0xDCu);
        PE_StoreU16(voice + 0x78u, (uint16_t)(PE_LoadU16(voice + 0x78u) - 1u));
        if ((new_value & 0xFF00u) != (old_value & 0xFF00u))
            akao_or_voice_pending(voice, 3u);
        PE_StoreU16(voice + 0x76u, (uint16_t)new_value);
    }

    if (PE_LoadU16(voice + 0x8Au))
        PE_StoreU16(voice + 0x8Au, (uint16_t)(PE_LoadU16(voice + 0x8Au) - 1u));
    if (PE_LoadU16(voice + 0x9Eu))
        PE_StoreU16(voice + 0x9Eu, (uint16_t)(PE_LoadU16(voice + 0x9Eu) - 1u));

    if (PE_LoadU16(voice + 0xBAu)) {
        PE_StoreU16(voice + 0xBAu, (uint16_t)(PE_LoadU16(voice + 0xBAu) - 1u));
        if (PE_LoadU16(voice + 0xBAu) == 0u) {
            pe_addr_t st = PE_LoadU32(0x8009D2C8u);
            PE_StoreU32(st + 0x34u, PE_LoadU32(st + 0x34u) ^ mask);
            PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x10u);
            akao_mark_audio_dirty();
        }
    }

    if (PE_LoadU16(voice + 0xBCu)) {
        PE_StoreU16(voice + 0xBCu, (uint16_t)(PE_LoadU16(voice + 0xBCu) - 1u));
        if (PE_LoadU16(voice + 0xBCu) == 0u) {
            pe_addr_t st = PE_LoadU32(0x8009D2C8u);
            PE_StoreU32(st + 0x3Cu, PE_LoadU32(st + 0x3Cu) ^ mask);
            akao_mark_audio_dirty();
        }
    }

    if (PE_LoadU16(voice + 0x96u)) {
        uint32_t base;
        PE_StoreU16(voice + 0x96u, (uint16_t)(PE_LoadU16(voice + 0x96u) - 1u));
        PE_StoreU16(voice + 0x94u,
                    (uint16_t)(PE_LoadU16(voice + 0x94u) + PE_LoadU16(voice + 0x98u)));
        value = (int)((PE_LoadU16(voice + 0x94u) & 0x7F00u) >> 8);
        base = PE_LoadU32(voice + 0x30u);
        if (PE_LoadU16(voice + 0x94u) & 0x8000u)
            scaled = (value * (int)base) >> 7;
        else
            scaled = (value * (int)(((base << 4) - base) >> 8)) >> 7;
        PE_StoreU16(voice + 0x92u, (uint16_t)scaled);

        if (PE_LoadU16(voice + 0x8Au) == 0u && PE_LoadU16(voice + 0x8Eu) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x1Cu));
            if (table) {
                value = ((int16_t)PE_LoadU16(voice + 0x92u) *
                         (int16_t)PE_LoadU16(table)) >>
                        16;
                if (value != (int16_t)PE_LoadU16(voice + 0xE8u)) {
                    PE_StoreU16(voice + 0xE8u, (uint16_t)value);
                    akao_or_voice_pending(voice, 0x10u);
                    if (value >= 0)
                        PE_StoreU16(voice + 0xE8u, (uint16_t)(value << 1));
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xA8u)) {
        PE_StoreU16(voice + 0xA8u, (uint16_t)(PE_LoadU16(voice + 0xA8u) - 1u));
        PE_StoreU16(voice + 0xA6u,
                    (uint16_t)(PE_LoadU16(voice + 0xA6u) + PE_LoadU16(voice + 0xAAu)));
        if (PE_LoadU16(voice + 0x9Eu) == 0u && PE_LoadU16(voice + 0xA2u) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x20u));
            if (table) {
                value = (((int16_t)PE_LoadU16(voice + 0x46u) *
                          (PE_LoadU16(voice + 0x6Cu) >> 8)) >>
                         7) *
                        (PE_LoadU16(voice + 0xA6u) >> 8);
                value = ((value << 9) >> 16) * (int16_t)PE_LoadU16(table);
                value >>= 15;
                if (value != (int16_t)PE_LoadU16(voice + 0xEAu)) {
                    PE_StoreU16(voice + 0xEAu, (uint16_t)value);
                    akao_or_voice_pending(voice, 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xB6u)) {
        PE_StoreU16(voice + 0xB6u, (uint16_t)(PE_LoadU16(voice + 0xB6u) - 1u));
        PE_StoreU16(voice + 0xB4u,
                    (uint16_t)(PE_LoadU16(voice + 0xB4u) + PE_LoadU16(voice + 0xB8u)));
        if (PE_LoadU16(voice + 0xB0u) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x24u));
            if (table) {
                value = ((PE_LoadU16(voice + 0xB4u) >> 8) *
                         (int16_t)PE_LoadU16(table)) >>
                        15;
                if (value != (int16_t)PE_LoadU16(voice + 0xECu)) {
                    PE_StoreU16(voice + 0xECu, (uint16_t)value);
                    akao_or_voice_pending(voice, 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0x7Au)) {
        old_value = PE_LoadU32(voice + 0x34u);
        new_value = old_value + PE_LoadU32(voice + 0x4Cu);
        PE_StoreU16(voice + 0x7Au, (uint16_t)(PE_LoadU16(voice + 0x7Au) - 1u));
        if ((new_value & 0xFFFF0000u) != (old_value & 0xFFFF0000u))
            akao_or_voice_pending(voice, 0x10u);
        PE_StoreU32(voice + 0x34u, new_value);
    }
}

/* Spu_TickVoiceEnvelopes — stream/CD bank voice ticker. */
void func_80087FA0(pe_addr_t voice, uint32_t mask)
{
    uint32_t old_value, new_value;
    int value, scaled;
    pe_addr_t table;

    if (PE_LoadU16(voice + 0x72u)) {
        old_value = PE_LoadU32(voice + 0x44u);
        new_value = old_value + PE_LoadU32(voice + 0x48u);
        PE_StoreU16(voice + 0x72u, (uint16_t)(PE_LoadU16(voice + 0x72u) - 1u));
        if ((new_value & 0xFFE00000u) != (old_value & 0xFFE00000u))
            akao_or_voice_pending(voice, 3u);
        PE_StoreU32(voice + 0x44u, new_value);
    }

    if (PE_LoadU16(voice + 0xBAu)) {
        PE_StoreU16(voice + 0xBAu, (uint16_t)(PE_LoadU16(voice + 0xBAu) - 1u));
        if (PE_LoadU16(voice + 0xBAu) == 0u) {
            PE_StoreU32(0x800BCD6Cu, PE_LoadU32(0x800BCD6Cu) ^ mask);
            PE_StoreU32(0x8009D2C4u, PE_LoadU32(0x8009D2C4u) | 0x10u);
            akao_mark_audio_dirty();
        }
    }

    if (PE_LoadU16(voice + 0xBCu)) {
        PE_StoreU16(voice + 0xBCu, (uint16_t)(PE_LoadU16(voice + 0xBCu) - 1u));
        if (PE_LoadU16(voice + 0xBCu) == 0u) {
            PE_StoreU32(0x800BCD74u, PE_LoadU32(0x800BCD74u) ^ mask);
            akao_mark_audio_dirty();
        }
    }

    if (PE_LoadU16(voice + 0x96u)) {
        uint32_t base;
        PE_StoreU16(voice + 0x96u, (uint16_t)(PE_LoadU16(voice + 0x96u) - 1u));
        PE_StoreU16(voice + 0x94u,
                    (uint16_t)(PE_LoadU16(voice + 0x94u) + PE_LoadU16(voice + 0x98u)));
        value = (int)((PE_LoadU16(voice + 0x94u) & 0x7F00u) >> 8);
        base = PE_LoadU32(voice + 0x30u);
        if (PE_LoadU16(voice + 0x94u) & 0x8000u)
            scaled = (value * (int)base) >> 7;
        else
            scaled = (value * (int)(((base << 4) - base) >> 8)) >> 7;
        PE_StoreU16(voice + 0x92u, (uint16_t)scaled);

        if (PE_LoadU16(voice + 0x8Eu) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x1Cu));
            if (table) {
                value = ((int16_t)PE_LoadU16(voice + 0x92u) *
                         (int16_t)PE_LoadU16(table)) >>
                        16;
                if (value != (int16_t)PE_LoadU16(voice + 0xE8u)) {
                    PE_StoreU16(voice + 0xE8u, (uint16_t)value);
                    akao_or_voice_pending(voice, 0x10u);
                    if (value >= 0)
                        PE_StoreU16(voice + 0xE8u, (uint16_t)(value << 1));
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xA8u)) {
        PE_StoreU16(voice + 0xA8u, (uint16_t)(PE_LoadU16(voice + 0xA8u) - 1u));
        PE_StoreU16(voice + 0xA6u,
                    (uint16_t)(PE_LoadU16(voice + 0xA6u) + PE_LoadU16(voice + 0xAAu)));
        if (PE_LoadU16(voice + 0xA2u) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x20u));
            if (table) {
                value = (((int16_t)PE_LoadU16(voice + 0x46u) *
                          (PE_LoadU16(voice + 0x6Cu) >> 8)) >>
                         7) *
                        (PE_LoadU16(voice + 0xA6u) >> 8);
                value = ((value << 9) >> 16) * (int16_t)PE_LoadU16(table);
                value >>= 15;
                if (value != (int16_t)PE_LoadU16(voice + 0xEAu)) {
                    PE_StoreU16(voice + 0xEAu, (uint16_t)value);
                    akao_or_voice_pending(voice, 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0xB6u)) {
        PE_StoreU16(voice + 0xB6u, (uint16_t)(PE_LoadU16(voice + 0xB6u) - 1u));
        PE_StoreU16(voice + 0xB4u,
                    (uint16_t)(PE_LoadU16(voice + 0xB4u) + PE_LoadU16(voice + 0xB8u)));
        if (PE_LoadU16(voice + 0xB0u) != 1u) {
            table = akao_wave_table(PE_LoadU32(voice + 0x24u));
            if (table) {
                value = ((PE_LoadU16(voice + 0xB4u) >> 8) *
                         (int16_t)PE_LoadU16(table)) >>
                        15;
                if (value != (int16_t)PE_LoadU16(voice + 0xECu)) {
                    PE_StoreU16(voice + 0xECu, (uint16_t)value);
                    akao_or_voice_pending(voice, 3u);
                }
            }
        }
    }

    if (PE_LoadU16(voice + 0x7Au)) {
        old_value = PE_LoadU32(voice + 0x34u);
        new_value = old_value + PE_LoadU32(voice + 0x4Cu);
        PE_StoreU16(voice + 0x7Au, (uint16_t)(PE_LoadU16(voice + 0x7Au) - 1u));
        if ((new_value & 0xFFFF0000u) != (old_value & 0xFFFF0000u))
            akao_or_voice_pending(voice, 0x10u);
        PE_StoreU32(voice + 0x34u, new_value);
    }
}

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

/* Exported retail names for host/tests. Bodies live as static helpers above. */
void func_8008900C(pe_addr_t voices, uint32_t active, uint32_t restart,
                   pe_addr_t key_on_out)
{
    uint32_t key = 0u;
    akao_step_voice_note(voices, active, restart, &key);
    if (key_on_out >= 0x80000000u && key_on_out < 0x80200000u)
        PE_StoreU32(key_on_out, key);
}

void func_80089F08(uint32_t voice_index, pe_addr_t env_out)
{
    akao_spu_get_voice_envelope(voice_index, env_out);
}

/* Guest leaf takes VoiceParams* (track+0xF0); host helper wants track base. */
void func_800878F0(int voice_index, pe_addr_t params)
{
    if (voice_index < 0 || !params)
        return;
    if (params >= 0xF0u)
        akao_write_voice_param((uint32_t)voice_index, params - 0xF0u);
}

/*
 * Akao_SetVoiceKeyOff — func_80088344. Still parked named-asm on the matching
 * side; empty host stub so the symbol stays resolvable. Score key-off flush
 * is func_80089784.
 */
void func_80088344(pe_addr_t voice, uint32_t voice_mask)
{
    (void)voice;
    (void)voice_mask;
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
