/*
 * Bounded PSX SPU voice mixer backed by pe_spu_dma's RAM/register image.
 */
#include "pe_spu_synth.h"
#include "pe_spu_dma.h"

#include <string.h>

#define SPU_KEY_ON_OFF  0x188u
#define SPU_KEY_OFF     0x18Au
#define SPU_VOICE_COUNT 24u
#define SPU_VOICE_STRIDE 0x10u

static const int16_t g_adpcm_filters[5][2] = {
    {0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}
};

typedef struct {
    int active;
    int16_t vol_l;
    int16_t vol_r;
    uint16_t pitch;
    uint32_t start_byte;
    uint32_t cur_byte;
    uint8_t nibble_high;
    int16_t hist1;
    int16_t hist2;
    uint32_t phase;
    int16_t sample;
    int16_t next_sample;
} PeSpuVoice;

static PeSpuVoice g_voices[SPU_VOICE_COUNT];
static unsigned g_frames_rendered;

static int16_t clamp_s16(int32_t v)
{
    if (v < -32768) return -32768;
    if (v > 32767) return 32767;
    return (int16_t)v;
}

static void load_block_header(PeSpuVoice *voice, uint32_t block_base)
{
    uint8_t header = PE_SpuRam_LoadU8(block_base);
    (void)header;
    voice->hist1 = 0;
    voice->hist2 = 0;
    voice->cur_byte = block_base + 16u;
    voice->nibble_high = 1u;
}

static int16_t decode_nibble(PeSpuVoice *voice, uint8_t shift, uint8_t filter,
                             uint8_t nibble)
{
    int32_t s = (int32_t)(nibble & 0xFu);
    if (s & 8) s -= 16;
    s <<= shift;
    s += (g_adpcm_filters[filter][0] * (int32_t)voice->hist1 +
          g_adpcm_filters[filter][1] * (int32_t)voice->hist2 + 32) >> 6;
    s = clamp_s16(s);
    voice->hist2 = voice->hist1;
    voice->hist1 = (int16_t)s;
    return (int16_t)s;
}

static int16_t read_adpcm_sample(PeSpuVoice *voice)
{
    if (voice->cur_byte >= PE_SPU_RAM_SIZE)
        return 0;

    uint32_t block_base = voice->cur_byte & ~0x7Fu;
    if ((voice->cur_byte & 0x7Fu) < 16u)
        load_block_header(voice, block_base);

    if (voice->cur_byte - block_base >= 128u) {
        load_block_header(voice, block_base);
    }

    uint8_t header = PE_SpuRam_LoadU8(block_base);
    uint8_t shift = (uint8_t)(12u - (header & 0xFu));
    uint8_t filter = (uint8_t)((header >> 4) & 7u);
    if (filter > 4u) filter = 0u;

    uint8_t b = PE_SpuRam_LoadU8(voice->cur_byte);
    uint8_t nibble = voice->nibble_high ? (uint8_t)(b >> 4) : (uint8_t)(b & 0xFu);
    voice->nibble_high = (uint8_t)!voice->nibble_high;
    if (voice->nibble_high)
        voice->cur_byte++;

    return decode_nibble(voice, shift, filter, nibble);
}

static void voice_key_on(unsigned index)
{
    if (index >= SPU_VOICE_COUNT) return;
    PeSpuVoice *v = &g_voices[index];
    uint32_t reg = index * SPU_VOICE_STRIDE;
    v->vol_l = (int16_t)PE_SpuRegister_LoadU16(reg);
    v->vol_r = (int16_t)PE_SpuRegister_LoadU16(reg + 2u);
    v->pitch = PE_SpuRegister_LoadU16(reg + 4u);
    v->start_byte = (uint32_t)PE_SpuRegister_LoadU16(reg + 6u) << 3;
    v->cur_byte = v->start_byte;
    v->nibble_high = 1u;
    v->hist1 = 0;
    v->hist2 = 0;
    v->phase = 0;
    v->sample = read_adpcm_sample(v);
    v->next_sample = read_adpcm_sample(v);
    v->active = v->pitch != 0u;
}

static void voice_key_off(unsigned index)
{
    if (index < SPU_VOICE_COUNT)
        g_voices[index].active = 0;
}

void PE_SpuSynth_Reset(void)
{
    memset(g_voices, 0, sizeof(g_voices));
    g_frames_rendered = 0;
}

void PE_SpuSynth_OnRegisterWrite(uint32_t offset, uint16_t value)
{
    if (offset == SPU_KEY_ON_OFF) {
        for (unsigned i = 0; i < 16u; i++) {
            if (value & (1u << i))
                voice_key_on(i);
        }
        return;
    }
    if (offset == SPU_KEY_OFF) {
        for (unsigned i = 0; i < 16u; i++) {
            if (value & (1u << i))
                voice_key_off(i);
        }
    }
}

static int16_t voice_step(PeSpuVoice *v)
{
    if (!v->active || v->pitch == 0u) return 0;

    v->phase += v->pitch;
    while (v->phase >= 0x1000u) {
        v->phase -= 0x1000u;
        v->sample = v->next_sample;
        v->next_sample = read_adpcm_sample(v);
    }

    int32_t frac = (int32_t)v->phase;
    int32_t delta = (int32_t)v->next_sample - (int32_t)v->sample;
    return (int16_t)(v->sample + ((delta * frac) >> 12));
}

void PE_SpuSynth_Render(int16_t *out_lr, unsigned frame_samples)
{
    unsigned f, i;
    if (!out_lr || frame_samples == 0u) return;

    for (f = 0; f < frame_samples; f++) {
        int32_t mix_l = 0, mix_r = 0;
        for (i = 0; i < SPU_VOICE_COUNT; i++) {
            PeSpuVoice *v = &g_voices[i];
            if (!v->active) continue;
            int16_t s = voice_step(v);
            mix_l += ((int32_t)s * (int32_t)v->vol_l) >> 15;
            mix_r += ((int32_t)s * (int32_t)v->vol_r) >> 15;
        }
        out_lr[f * 2u] = clamp_s16(mix_l);
        out_lr[f * 2u + 1u] = clamp_s16(mix_r);
    }
    g_frames_rendered++;
}

uint64_t PE_SpuSynth_HashMix(unsigned frame_samples)
{
    int16_t buf[PE_SPU_SYNTH_FRAME_SAMPLES * 2u];
    uint64_t h = 0x9E3779B97F4A7C15ULL;
    unsigned n = frame_samples;
    if (n > PE_SPU_SYNTH_FRAME_SAMPLES) n = PE_SPU_SYNTH_FRAME_SAMPLES;
    PE_SpuSynth_Render(buf, n);
    for (unsigned i = 0; i < n * 2u; i++) {
        h ^= (uint64_t)(uint16_t)buf[i] + ((uint64_t)i << 32);
        h *= UINT64_C(1099511628211);
    }
    return h;
}

unsigned PE_SpuSynth_ActiveVoiceCount(void)
{
    unsigned n = 0;
    for (unsigned i = 0; i < SPU_VOICE_COUNT; i++)
        if (g_voices[i].active) n++;
    return n;
}

unsigned PE_SpuSynth_FramesRendered(void)
{
    return g_frames_rendered;
}
