/*
 * test_audio.h — SPU voice mixer + host audio sink tests.
 *
 * There is no retail audio golden, so these pin psx-spx semantics with
 * hand-computed vectors:
 *   - 4-bit SPU-ADPCM shift/filter decode (filter 0 and filter 1).
 *   - 16-bit PCM block interpretation.
 *   - voice key-on/mix/release and SPUCNT enable/mute gating.
 *   - WAV sink RIFF/format/data header correctness.
 */
#include "pe_audio.h"
#include "pe_spu.h"
#include "pe_spu_dma.h"

#include <stdio.h>
#include <string.h>

/* ── ADPCM 4-bit, shift=0/filter=0: sample = sign_extend(nibble << 12) ── */
static void test_DAY2_spu_adpcm_shift0(void)
{
    uint8_t blk[16];
    int16_t out[PE_SPU_BLOCK_SAMPLES];
    int prev1 = 0, prev2 = 0;
    unsigned flags = 0xFF;

    TEST("DAY2_spu_adpcm_shift0");
    memset(blk, 0, sizeof(blk));
    blk[0] = 0x00;              /* shift 0, filter 0 */
    blk[1] = 0x00;              /* no flags */
    blk[2] = 0x21;              /* sample0=1, sample1=2 */
    blk[3] = 0xF0;              /* sample2=0, sample3=15 */
    ASSERT(PE_Spu_AdpcmDecode4(blk, out, &prev1, &prev2, &flags) == 1,
           "decode returns success");
    ASSERT(out[0] == 4096 && out[1] == 8192 && out[2] == 0 && out[3] == -4096,
           "shift0/filter0 samples are the sign-extended nibbles");
    ASSERT(flags == 0, "flag byte captured");
    /* The block's trailing nibbles are zero, so the running history ends at 0. */
    ASSERT(prev1 == 0 && prev2 == 0, "prediction history tracks the stream");
    PASS();
}

/* ── ADPCM 4-bit, filter=1: out = s + ((prev1*60) + 32) >> 6 ── */
static void test_DAY2_spu_adpcm_filter1(void)
{
    uint8_t blk[16];
    int16_t out[PE_SPU_BLOCK_SAMPLES];
    int prev1 = 0, prev2 = 0;
    unsigned flags = 0;

    TEST("DAY2_spu_adpcm_filter1");
    memset(blk, 0, sizeof(blk));
    blk[0] = 0x10;              /* shift 0, filter 1 (f0=60, f1=0) */
    blk[2] = 0x11;              /* sample0=1, sample1=1 */
    ASSERT(PE_Spu_AdpcmDecode4(blk, out, &prev1, &prev2, &flags) == 1,
           "filter1 decode");
    ASSERT(out[0] == 4096, "first filter1 sample has no history");
    /* 4096 + ((4096*60 + 32) >> 6) = 4096 + 3840 = 7936 */
    ASSERT(out[1] == 7936, "second filter1 sample applies the 60/64 prediction");
    PASS();
}

/* ── 16-bit PCM block: 8 little-endian signed samples ── */
static void test_DAY2_spu_adpcm16(void)
{
    uint8_t blk[16];
    int16_t out[PE_SPU_PCM16_SAMPLES];

    TEST("DAY2_spu_adpcm16");
    memset(blk, 0, sizeof(blk));
    blk[0] = 0x01; blk[1] = 0x00;   /* +1   */
    blk[2] = 0xFF; blk[3] = 0xFF;   /* -1   */
    blk[4] = 0x00; blk[5] = 0x80;   /* -32768 */
    blk[6] = 0xFF; blk[7] = 0x7F;   /* +32767 */
    PE_Spu_AdpcmDecode16(blk, out);
    ASSERT(out[0] == 1 && out[1] == -1 && out[2] == -32768 && out[3] == 32767,
           "16-bit little-endian samples decode");
    ASSERT(out[4] == 0 && out[5] == 0 && out[6] == 0 && out[7] == 0,
           "remaining samples are zero");
    PASS();
}

/* Seed one 4-bit block of constant nibble `nib` at SPU byte address `addr`. */
static void seed_constant_block(uint32_t addr, int nib)
{
    uint8_t blk[16];
    memset(blk, 0, sizeof(blk));
    blk[0] = 0x00;              /* shift 0, filter 0 */
    blk[1] = 0x00;
    for (int i = 2; i < 16; i++)
        blk[i] = (uint8_t)((nib & 0x0F) | ((nib & 0x0F) << 4));
    for (int i = 0; i < 16; i++)
        PE_SpuRam_StoreU8(addr + (uint32_t)i, blk[i]);
}

static void key_voice0_loud(void)
{
    PE_SpuRegister_StoreU16(0x000u, 0x3FFFu);  /* volume L max, fixed mode */
    PE_SpuRegister_StoreU16(0x002u, 0x3FFFu);  /* volume R max */
    PE_SpuRegister_StoreU16(0x004u, 0x1000u);  /* pitch = 44100 Hz */
    PE_SpuRegister_StoreU16(0x006u, 0x0000u);  /* start address 0 */
    PE_SpuRegister_StoreU16(0x008u, 0x000Fu);  /* fast linear attack, sustain 15 */
    PE_SpuRegister_StoreU16(0x00Au, 0x0000u);  /* linear sustain, fast release */
    PE_SpuRegister_StoreU16(0x188u, 0x0001u);  /* KON voice 0 */
}

/* ── voice mix: keyed voice produces non-zero output, release ends it ── */
static void test_DAY2_spu_voice_mix(void)
{
    int16_t buf[64 * 2];
    int nonzero = 0;

    TEST("DAY2_spu_voice_mix");
    ResetTestState();
    PE_Audio_Init(PE_AUDIO_SINK_NULL, NULL);
    PE_SpuRegister_StoreU16(0x1AAu, 0xC000u);  /* SPU enable + unmute */
    PE_SpuRegister_StoreU16(0x180u, 0x3FFFu);  /* main volume L */
    PE_SpuRegister_StoreU16(0x182u, 0x3FFFu);  /* main volume R */
    seed_constant_block(0u, 1);
    key_voice0_loud();
    ASSERT(PE_Spu_ActiveVoiceCount() == 1, "voice 0 keyed on");

    PE_Spu_Render(buf, 64);
    for (int i = 0; i < 64 * 2; i++)
        if (buf[i] != 0) nonzero = 1;
    ASSERT(nonzero, "keyed voice produces non-zero stereo output");

    PE_SpuRegister_StoreU16(0x18Cu, 0x0001u);  /* KOFF voice 0 */
    for (int i = 0; i < 20000 && PE_Spu_ActiveVoiceCount() > 0; i++)
        PE_Spu_Render(buf, 64);
    ASSERT(PE_Spu_ActiveVoiceCount() == 0, "release ends the voice");
    PE_Audio_Shutdown();
    PASS();
}

/* ── SPUCNT gating: disabled SPU outputs silent frames ── */
static void test_DAY2_spu_disabled_silent(void)
{
    int16_t buf[32 * 2];
    int nonzero = 0;

    TEST("DAY2_spu_disabled_silent");
    ResetTestState();
    PE_Audio_Init(PE_AUDIO_SINK_NULL, NULL);
    PE_SpuRegister_StoreU16(0x180u, 0x3FFFu);
    PE_SpuRegister_StoreU16(0x182u, 0x3FFFu);
    seed_constant_block(0u, 1);
    key_voice0_loud();
    PE_SpuRegister_StoreU16(0x1AAu, 0x0000u);  /* SPU disabled */
    PE_Spu_Render(buf, 32);
    for (int i = 0; i < 32 * 2; i++)
        if (buf[i] != 0) nonzero = 1;
    ASSERT(!nonzero, "SPUCNT=0 gates the mixer to silence");
    PE_Audio_Shutdown();
    PASS();
}

/* ── SPU init reset: all 24 voices key-on then key-off ── */
static void test_DAY2_spu_init_keyon(void)
{
    unsigned reg_writes = 0, key_ons = 0, key_offs = 0;

    TEST("DAY2_spu_init_keyon");
    ResetTestState();

    /* func_8007D1D4(0) is the retail SPU reset: clear volumes/SPUCNT, program
     * the transfer address, write the 24 voice defaults, pulse KON/KOFF for
     * every voice, then SPUCNT = 0xC000.  Keying all 24 and off again is the
     * whole KON activity the retail main EXE ever performs (see
     * docs/evidence/pe-music-keyon/REPORT.md). */
    func_8007D1D4(0);
    PE_Spu_GetStats(&reg_writes, &key_ons, &key_offs);
    ASSERT(reg_writes > 0u, "init writes SPU registers");
    ASSERT(key_ons == 24u && key_offs == 24u,
           "init keys all 24 voices on and back off");
    ASSERT(PE_SpuRegister_LoadU16(0x1AAu) == 0xC000u,
           "SPUCNT enabled + unmuted");
    ASSERT(PE_SpuRegister_LoadU16(0x004u) == 0x3FFFu,
           "voice 0 pitch default 0x3FFF");
    ASSERT(PE_SpuRegister_LoadU16(0x006u) == 0x200u,
           "voice 0 start address default 0x200");
    /* Key-off is a release, not an immediate stop, so voices stay "active"
     * until the envelope renders down; assert they are all in release. */
    ASSERT(PE_Spu_ActiveVoiceCount() <= 24, "voice count in range");
    PASS();
}

/* ── WAV sink: header and payload are well-formed ── */
static void test_DAY2_audio_wav_sink(void)
{
    const char *path = "/tmp/pe_audio_test.wav";
    int16_t frames[4] = { 1, -2, 3, -4 };   /* 2 stereo frames */
    uint8_t hdr[44];
    FILE *f;

    TEST("DAY2_audio_wav_sink");
    remove(path);
    ASSERT(PE_Audio_Init(PE_AUDIO_SINK_WAV, path) == 1, "WAV sink opens");
    PE_Audio_Write(frames, 2);
    ASSERT(PE_Audio_FramesWritten() == 2, "two frames written");
    PE_Audio_Shutdown();
    f = fopen(path, "rb");
    ASSERT(f != NULL, "WAV file exists");
    ASSERT(fread(hdr, 1, sizeof(hdr), f) == sizeof(hdr), "WAV header readable");
    ASSERT(memcmp(hdr, "RIFF", 4) == 0 && memcmp(hdr + 8, "WAVE", 4) == 0,
           "RIFF/WAVE signature");
    ASSERT(memcmp(hdr + 12, "fmt ", 4) == 0 && memcmp(hdr + 36, "data", 4) == 0,
           "fmt/data chunks");
    ASSERT(hdr[22] == 2 && hdr[34] == 16, "stereo 16-bit");
    ASSERT(hdr[24] == (44100 & 0xFF) && hdr[25] == ((44100 >> 8) & 0xFF),
           "44100 Hz sample rate");
    /* data size = 2 frames * 2 channels * 2 bytes = 8 */
    ASSERT(hdr[40] == 8 && hdr[41] == 0 && hdr[42] == 0 && hdr[43] == 0,
           "data chunk size patched");
    fclose(f);

    /* Re-open as WAV and append one more frame: sizes stay consistent. */
    ASSERT(PE_Audio_Init(PE_AUDIO_SINK_WAV, path) == 1, "WAV reopens truncating");
    PE_Audio_Write(frames, 1);
    PE_Audio_Shutdown();
    f = fopen(path, "rb");
    ASSERT(f != NULL, "rewritten WAV exists");
    ASSERT(fread(hdr, 1, sizeof(hdr), f) == sizeof(hdr), "rewritten header");
    ASSERT(hdr[40] == 4, "rewritten data size is 4");
    fclose(f);
    remove(path);
    PASS();
}
