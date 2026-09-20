/*
 * pe_spu.c — 24-voice SPU mixer (see pe_spu.h for scope and named gaps).
 */
#include "pe_spu.h"
#include "pe_audio.h"
#include "pe_spu_dma.h"

#include <string.h>

/* SPU register offsets (relative to 0x1F801C00). */
#define SPU_MAIN_VOL_L   0x180u
#define SPU_MAIN_VOL_R   0x182u
#define SPU_KON_LO       0x188u
#define SPU_KON_HI       0x18Au
#define SPU_KOFF_LO      0x18Cu
#define SPU_KOFF_HI      0x18Eu
#define SPU_ENDX_LO      0x19Cu
#define SPU_SPUCNT       0x1AAu

#define SPU_VOICE_BASE(v) ((uint32_t)(v) * 0x10u)
#define SPU_VOL_L(v)      (SPU_VOICE_BASE(v) + 0x0u)
#define SPU_VOL_R(v)      (SPU_VOICE_BASE(v) + 0x2u)
#define SPU_PITCH(v)      (SPU_VOICE_BASE(v) + 0x4u)
#define SPU_START(v)      (SPU_VOICE_BASE(v) + 0x6u)
#define SPU_ADSR_LO(v)    (SPU_VOICE_BASE(v) + 0x8u)
#define SPU_ADSR_HI(v)    (SPU_VOICE_BASE(v) + 0xAu)
#define SPU_REPEAT(v)     (SPU_VOICE_BASE(v) + 0xEu)

#define SPU_RAM_MASK (PE_SPU_RAM_SIZE - 1u)

/* One vblank at 44100 Hz / 60 Hz.  The port's frame model is 60 Hz. */
#define PE_AUDIO_SAMPLES_PER_VBLANK 735u

enum { ENV_OFF = 0, ENV_ATTACK = 1, ENV_DECAY = 2, ENV_SUSTAIN = 3, ENV_RELEASE = 4 };

typedef struct PeSpuVoice {
    int      index;
    int      active;
    int      phase;          /* ENV_* */
    int      env_level;      /* 0..0x7FFF */
    uint32_t env_counter;
    uint32_t curr_addr;      /* current ADPCM block byte address */
    uint32_t repeat_addr;    /* loop target, bytes */
    int      prev1, prev2;   /* ADPCM prediction history */
    uint32_t counter;        /* pitch counter (12.20-ish; bit12 selects sample) */
    int      played;         /* decoded samples consumed */
    int      block_pos;      /* next index into block[] */
    int16_t  block[PE_SPU_BLOCK_SAMPLES];
    unsigned block_flags;    /* bit0 end, bit1 repeat, bit2 loop-start */
    int16_t  hist[4];        /* oldest .. newest decoded samples */
} PeSpuVoice;

static PeSpuVoice g_voices[PE_SPU_VOICE_COUNT];
static int16_t g_audio_buf[PE_AUDIO_SAMPLES_PER_VBLANK * PE_AUDIO_CHANNELS];

/* Diagnostics: how much the guest actually drove the SPU. */
static unsigned g_stat_reg_writes, g_stat_key_ons, g_stat_key_offs;

void PE_Spu_GetStats(unsigned *reg_writes, unsigned *key_ons, unsigned *key_offs)
{
    if (reg_writes) *reg_writes = g_stat_reg_writes;
    if (key_ons)    *key_ons    = g_stat_key_ons;
    if (key_offs)   *key_offs   = g_stat_key_offs;
}

/* ── psx-spx SPU-ADPCM prediction coefficients (positive prev1 / negative) ── */
static const int kPosF0[5] = { 0,  60, 115,  98, 122 };
static const int kPosF1[5] = { 0,   0, -52, -55, -60 };
static const int kNegF0[5] = { 0,   0, -60, -115, -98 };
static const int kNegF1[5] = { 0,   0,   0,  52,  55 };

int PE_Spu_AdpcmDecode4(const uint8_t block[PE_SPU_BLOCK_BYTES],
                        int16_t out[PE_SPU_BLOCK_SAMPLES],
                        int *prev1, int *prev2, unsigned *flags_out)
{
    int shift, filter, i;
    int p1 = prev1 ? *prev1 : 0;
    int p2 = prev2 ? *prev2 : 0;

    if (!block || !out) return 0;
    shift  = block[0] & 0x0Fu;
    filter = (block[0] >> 4) & 0x0Fu;
    if (filter > 4) filter = 4;
    if (flags_out) *flags_out = (unsigned)(block[1] & 0x07u);

    for (i = 0; i < PE_SPU_BLOCK_SAMPLES; i++) {
        int byte = block[2 + (i >> 1)];
        int nib  = (i & 1) ? ((byte >> 4) & 0x0F) : (byte & 0x0F);
        int s    = (int)(int16_t)(nib << 12);   /* sign-extend the 4-bit nibble */
        int f0, f1;

        s >>= shift;                             /* arithmetic, 0..15 */
        if (p1 >= 0) { f0 = kPosF0[filter]; f1 = kPosF1[filter]; }
        else         { f0 = kNegF0[filter]; f1 = kNegF1[filter]; }
        s += (p1 * f0 + p2 * f1 + 32) >> 6;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        out[i] = (int16_t)s;
        p2 = p1;
        p1 = s;
    }
    if (prev1) *prev1 = p1;
    if (prev2) *prev2 = p2;
    return 1;
}

void PE_Spu_AdpcmDecode16(const uint8_t block[PE_SPU_BLOCK_BYTES],
                          int16_t out[PE_SPU_PCM16_SAMPLES])
{
    int i;
    if (!block || !out) return;
    for (i = 0; i < PE_SPU_PCM16_SAMPLES; i++)
        out[i] = (int16_t)((uint16_t)block[i * 2] |
                           ((uint16_t)block[i * 2 + 1] << 8));
}

static void set_endx(int voice, int on)
{
    uint16_t e = PE_SpuRegister_LoadU16(SPU_ENDX_LO);
    if (on) e |= (uint16_t)(1u << voice);
    else    e &= (uint16_t)~(1u << voice);
    PE_SpuRegister_StoreU16(SPU_ENDX_LO, e);
}

void PE_Spu_Reset(void)
{
    memset(g_voices, 0, sizeof(g_voices));
    g_stat_reg_writes = g_stat_key_ons = g_stat_key_offs = 0;
    for (int i = 0; i < PE_SPU_VOICE_COUNT; i++) {
        g_voices[i].index = i;
        g_voices[i].block_pos = PE_SPU_BLOCK_SAMPLES; /* force a load */
    }
}

/* Decode the 16-byte block at `addr` (already masked) into v->block and update
 * the prediction history.  Loop-start flags latch the repeat address. */
static void voice_load_block(PeSpuVoice *v, uint32_t addr)
{
    uint8_t blk[PE_SPU_BLOCK_BYTES];
    unsigned flags = 0;

    addr &= SPU_RAM_MASK;
    for (int i = 0; i < PE_SPU_BLOCK_BYTES; i++)
        blk[i] = PE_SpuRam_LoadU8((addr + (uint32_t)i) & SPU_RAM_MASK);
    PE_Spu_AdpcmDecode4(blk, v->block, &v->prev1, &v->prev2, &flags);
    v->block_flags = flags;
    v->block_pos   = 0;
    v->curr_addr   = addr;
    if (flags & 0x04u) v->repeat_addr = addr;   /* Loop Start: latch repeat addr */
}

/* Return the next decoded sample, crossing/looping blocks as needed. */
static int voice_next_decoded_sample(PeSpuVoice *v)
{
    if (v->block_pos >= PE_SPU_BLOCK_SAMPLES) {
        unsigned flags = v->block_flags;
        uint32_t next;
        if (flags & 0x01u) {                    /* Loop End */
            set_endx(v->index, 1);
            next = v->repeat_addr;
            if (!(flags & 0x02u)) {             /* End+Mute: release, env=0 */
                v->phase = ENV_RELEASE;
                v->env_level = 0;
            }
        } else {
            next = (v->curr_addr + PE_SPU_BLOCK_BYTES) & SPU_RAM_MASK;
        }
        voice_load_block(v, next);
    }
    return v->block[v->block_pos++];
}

/* Advance the pitch counter and linearly interpolate the two most recent
 * samples.  (The hardware's 4-point gaussian is a documented gap.) */
static int32_t voice_fetch(PeSpuVoice *v)
{
    uint32_t idx  = v->counter >> 12;
    int      frac = (int)((v->counter >> 4) & 0xFFu);
    int32_t  a, b;

    while (v->played <= (int)idx) {
        int s = voice_next_decoded_sample(v);
        v->hist[0] = v->hist[1];
        v->hist[1] = v->hist[2];
        v->hist[2] = v->hist[3];
        v->hist[3] = (int16_t)s;
        v->played++;
        if (v->played > 1 << 20) break;         /* defensive */
    }
    a = v->hist[2];
    b = v->hist[3];
    return a + ((b - a) * frac) / 256;
}

static void voice_key_on(int index)
{
    PeSpuVoice *v = &g_voices[index];
    uint32_t start8 = PE_SpuRegister_LoadU16(SPU_START(index));

    g_stat_key_ons++;
    v->curr_addr   = (start8 << 3) & SPU_RAM_MASK;
    v->repeat_addr = v->curr_addr;
    v->prev1 = v->prev2 = 0;
    v->counter = 0;
    v->played = 0;
    v->block_pos = 0;
    v->block_flags = 0;
    v->hist[0] = v->hist[1] = v->hist[2] = v->hist[3] = 0;
    v->phase = ENV_ATTACK;
    v->env_level = 0;
    v->env_counter = 0;
    v->active = 1;
    set_endx(index, 0);
    /* Decode the block at the start address now so the first fetched sample is
     * the sample's first sample (a lazy "block ended -> advance" would skip it). */
    voice_load_block(v, v->curr_addr);
}

static void voice_key_off(int index)
{
    PeSpuVoice *v = &g_voices[index];
    if (!v->active) return;
    g_stat_key_offs++;
    v->phase = ENV_RELEASE;
}

void PE_Spu_OnRegisterWrite(uint32_t offset, uint16_t value)
{
    g_stat_reg_writes++;
    if (offset == SPU_KON_LO || offset == SPU_KON_HI) {
        int base = (offset == SPU_KON_LO) ? 0 : 16;
        for (int b = 0; b < 16; b++)
            if (value & (uint16_t)(1u << b)) voice_key_on(base + b);
    } else if (offset == SPU_KOFF_LO || offset == SPU_KOFF_HI) {
        int base = (offset == SPU_KOFF_LO) ? 0 : 16;
        for (int b = 0; b < 16; b++)
            if (value & (uint16_t)(1u << b)) voice_key_off(base + b);
    }
}

/* One ADSR step per 44.1 kHz cycle, psx-spx "Envelope Operation". */
static void envelope_apply(PeSpuVoice *v, int shift, int step_value,
                           int exponential, int decreasing)
{
    int adsr_step = 7 - step_value;
    uint32_t inc;
    uint32_t counter;

    if (decreasing) adsr_step = ~adsr_step;          /* +7..+4 => -8..-5 */
    if (shift > 11) adsr_step <<= 0;
    else            adsr_step <<= (11 - shift);
    inc = (shift > 11) ? (0x8000u >> (shift - 11)) : 0x8000u;

    if (exponential && !decreasing && v->env_level > 0x6000) {
        if (shift < 10) adsr_step /= 4;
        else if (shift >= 11) inc /= 4;
        else { adsr_step /= 2; inc /= 2; }
    } else if (exponential && decreasing) {
        adsr_step = (adsr_step * v->env_level) / 0x8000;
    }

    counter = v->env_counter + inc;
    v->env_counter = counter;
    if (!(counter & 0x8000u)) return;
    v->env_level += adsr_step;
}

static void voice_envelope_tick(PeSpuVoice *v)
{
    uint16_t lo = PE_SpuRegister_LoadU16(SPU_ADSR_LO(v->index));
    uint16_t hi = PE_SpuRegister_LoadU16(SPU_ADSR_HI(v->index));

    switch (v->phase) {
    case ENV_ATTACK: {
        int mode  = (lo >> 15) & 1;
        int shift = (lo >> 10) & 0x1F;
        int step  = (lo >> 8) & 3;
        envelope_apply(v, shift, step, mode, 0);
        if (v->env_level >= 0x7FFF) { v->env_level = 0x7FFF; v->phase = ENV_DECAY; }
        break;
    }
    case ENV_DECAY: {
        int shift = (lo >> 4) & 0x0F;
        int sl    = ((lo & 0x0F) + 1) * 0x800;
        envelope_apply(v, shift, 0, 1, 1);           /* always exponential */
        if (v->env_level <= sl) { v->env_level = sl; v->phase = ENV_SUSTAIN; }
        break;
    }
    case ENV_SUSTAIN: {
        int mode  = (hi >> 15) & 1;
        int dir   = (hi >> 14) & 1;
        int shift = (hi >> 8) & 0x1F;
        int step  = (hi >> 6) & 3;
        envelope_apply(v, shift, step, mode, dir);
        if (v->env_level < 0) v->env_level = 0;
        if (v->env_level > 0x7FFF) v->env_level = 0x7FFF;
        break;
    }
    case ENV_RELEASE: {
        int mode  = (hi >> 5) & 1;
        int shift = hi & 0x1F;
        envelope_apply(v, shift, 0, mode, 1);
        if (v->env_level <= 0) {
            v->env_level = 0;
            v->phase = ENV_OFF;
            v->active = 0;
        }
        break;
    }
    default:
        break;
    }
}

static int16_t clamp16(int32_t v)
{
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

/* Fixed-mode volume register: bit15=0, bits0-14 are half the signed volume. */
static int voice_volume_gain(uint16_t reg)
{
    if (reg & 0x8000u) return 0;            /* sweep mode: not swept (gap) */
    return (int)(int16_t)(uint16_t)(reg << 1);
}

void PE_Spu_Render(int16_t *stereo, unsigned frames)
{
    uint16_t spucnt;
    int enabled, mvl, mvr;

    if (!stereo) return;
    spucnt  = PE_SpuRegister_LoadU16(SPU_SPUCNT);
    enabled = ((spucnt & 0x8000u) != 0u) && ((spucnt & 0x4000u) != 0u);
    mvl = voice_volume_gain(PE_SpuRegister_LoadU16(SPU_MAIN_VOL_L));
    mvr = voice_volume_gain(PE_SpuRegister_LoadU16(SPU_MAIN_VOL_R));

    for (unsigned n = 0; n < frames; n++) {
        int32_t mixl = 0, mixr = 0;

        if (enabled) {
            for (int i = 0; i < PE_SPU_VOICE_COUNT; i++) {
                PeSpuVoice *v = &g_voices[i];
                int32_t s;
                int gl, gr;
                uint16_t pitch, vlo, vro;

                if (!v->active) continue;
                voice_envelope_tick(v);
                if (!v->active) continue;

                pitch = PE_SpuRegister_LoadU16(SPU_PITCH(i));
                if (pitch == 0u) continue;           /* pitch 0 = stopped */
                v->counter += (pitch > 0x4000u) ? 0x4000u : pitch;

                s = voice_fetch(v);
                s = (s * v->env_level) >> 15;
                vlo = PE_SpuRegister_LoadU16(SPU_VOL_L(i));
                vro = PE_SpuRegister_LoadU16(SPU_VOL_R(i));
                gl = voice_volume_gain(vlo);
                gr = voice_volume_gain(vro);
                mixl += (s * gl) >> 15;
                mixr += (s * gr) >> 15;
            }
        }
        mixl = (mixl * mvl) >> 15;
        mixr = (mixr * mvr) >> 15;
        stereo[n * 2 + 0] = clamp16(mixl);
        stereo[n * 2 + 1] = clamp16(mixr);
    }
}

int PE_Spu_ActiveVoiceCount(void)
{
    int n = 0;
    for (int i = 0; i < PE_SPU_VOICE_COUNT; i++)
        if (g_voices[i].active) n++;
    return n;
}

void PE_Spu_RenderVBlank(void)
{
    PE_Spu_Render(g_audio_buf, PE_AUDIO_SAMPLES_PER_VBLANK);
    if (PE_Audio_IsOpen())
        PE_Audio_Write(g_audio_buf, PE_AUDIO_SAMPLES_PER_VBLANK);
}

/* ── pipeline self-test (synthetic; NOT retail audio) ──────────────────────
 * Seeds an audible looped SPU-ADPCM square tone directly through the real
 * mixer/sink path so a live-run WAV can be heard end to end.  The guest music
 * sequencer does not yet key voices, so this is the only currently audible
 * artifact; it is clearly a test signal, not a claim of retail audio parity. */
static void selftest_seed_block(uint32_t addr, int first, int last)
{
    uint8_t blk[PE_SPU_BLOCK_BYTES];
    int block_index = (int)(addr / PE_SPU_BLOCK_BYTES);
    int base_sample = block_index * PE_SPU_BLOCK_SAMPLES;

    memset(blk, 0, sizeof(blk));
    blk[0] = 0x00;                     /* shift 0, filter 0 (raw nibbles) */
    blk[1] = (uint8_t)((first ? 0x04 : 0x00) | (last ? 0x03 : 0x00));
    for (int i = 0; i < PE_SPU_BLOCK_SAMPLES; i++) {
        int idx = base_sample + i;
        int value = ((idx / PE_SPU_BLOCK_SAMPLES) & 1) ? -7 : 7;  /* ~787 Hz */
        int nib = value & 0x0F;
        int bi = 2 + (i >> 1);
        if (i & 1) blk[bi] |= (uint8_t)(nib << 4);
        else       blk[bi] |= (uint8_t)nib;
    }
    for (int i = 0; i < PE_SPU_BLOCK_BYTES; i++)
        PE_SpuRam_StoreU8(addr + (uint32_t)i, blk[i]);
}

void PE_Spu_SelfTest(unsigned frames)
{
    PE_Spu_Reset();
    PE_SpuRegister_StoreU16(SPU_SPUCNT, 0xC000u);      /* enable + unmute */
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_L, 0x3FFFu);
    PE_SpuRegister_StoreU16(SPU_MAIN_VOL_R, 0x3FFFu);
    for (int i = 0; i < 4; i++)
        selftest_seed_block((uint32_t)i * PE_SPU_BLOCK_BYTES, i == 0, i == 3);
    PE_SpuRegister_StoreU16(SPU_VOL_L(0), 0x3FFFu);
    PE_SpuRegister_StoreU16(SPU_VOL_R(0), 0x3FFFu);
    PE_SpuRegister_StoreU16(SPU_PITCH(0), 0x1000u);
    PE_SpuRegister_StoreU16(SPU_START(0), 0x0000u);
    PE_SpuRegister_StoreU16(SPU_ADSR_LO(0), 0x000Fu);
    PE_SpuRegister_StoreU16(SPU_ADSR_HI(0), 0x0000u);
    PE_SpuRegister_StoreU16(SPU_KON_LO, 0x0001u);

    while (frames > 0u) {
        unsigned n = frames > PE_AUDIO_SAMPLES_PER_VBLANK
                     ? PE_AUDIO_SAMPLES_PER_VBLANK : frames;
        PE_Spu_Render(g_audio_buf, n);
        if (PE_Audio_IsOpen())
            PE_Audio_Write(g_audio_buf, n);
        frames -= n;
    }
    PE_SpuRegister_StoreU16(SPU_KOFF_LO, 0x0001u);
    /* Leave the SPU in a clean state for the guest boot path. */
    PE_SpuDma_Reset();
    PE_Spu_Reset();
}
