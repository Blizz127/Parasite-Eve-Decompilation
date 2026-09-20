/*
 * test_xa.h — unit vectors for the CD-XA ADPCM decoder + resampler (pe_xa.c).
 *
 * The vectors are hand-computed from the decode algorithm (psx-spx filter
 * tables + the reference sound-group layout) and from the psx-spx 512-entry
 * gaussian interpolation table:
 *   - 4-bit decode: param 0 => shift=0/filter=0, a lone nibble N is (int16)(N<<12).
 *   - 8-bit decode: parameter bytes at +4, 28 LE 32-bit words at +16; byte N of
 *     each word is sub-block N's sample (the pre-change code read +8 as data).
 *   - gaussian taps: interp(oldest..newest, i=0) with a 32767 impulse gives
 *     (table[0FFh]*A)>>15 etc.; hard-coded psx-spx table entries.
 *   - volume chain: ATV0..3 matrix, SPU CD input volume, SPU main volume and
 *     the SPUCNT CD-audio-enable gate (all were ignored before this change).
 */
#include "pe_xa.h"
#include "pe_cdreg.h"
#include "pe_spu_dma.h"

#include <string.h>

/* ── helpers ────────────────────────────────────────────────────────────── */

/* Retail-like CD-XA output path: identity ATV matrix, CD/main volume max,
 * SPUCNT enabled + unmuted + CD audio enabled (func_8007BAC0 shape). */
static void xa_set_atv(unsigned a0, unsigned a1, unsigned a2, unsigned a3)
{
    PE_CdReg_WriteU8(0x1F801800u, 2u);
    PE_CdReg_WriteU8(0x1F801802u, (uint8_t)a0);
    PE_CdReg_WriteU8(0x1F801803u, (uint8_t)a1);
    PE_CdReg_WriteU8(0x1F801800u, 3u);
    PE_CdReg_WriteU8(0x1F801801u, (uint8_t)a2);
    PE_CdReg_WriteU8(0x1F801802u, (uint8_t)a3);
    PE_CdReg_WriteU8(0x1F801803u, 0x20u);
}

static void xa_audio_path_default(void)
{
    PE_CdReg_Reset();
    xa_set_atv(0x80u, 0u, 0x80u, 0u);
    PE_SpuRegister_StoreU16(0x1AAu, 0xC001u);  /* enable + unmute + CD audio */
    PE_SpuRegister_StoreU16(0x180u, 0x3FFFu);
    PE_SpuRegister_StoreU16(0x182u, 0x3FFFu);
    PE_SpuRegister_StoreU16(0x1B0u, 0x3FFFu);
    PE_SpuRegister_StoreU16(0x1B2u, 0x3FFFu);
}

/* One Form-2 XA audio sector, all 18 groups identical.  4-bit: sub-blocks
 * 0,2,4,6 = nib_l (left), 1,3,5,7 = nib_r (right); mono uses nib_all. */
static void xa_build_4bit_sector(uint8_t *sector, int stereo, int rate18900,
                                 int nib_all, int nib_l, int nib_r)
{
    memset(sector, 0, 2352);
    sector[18] = 0x64;                          /* Form-2, audio submode bit2 */
    sector[19] = (uint8_t)((stereo ? 1u : 0u) | (rate18900 ? 2u : 0u));
    for (int p = 0; p < 18; p++) {
        uint8_t *g = sector + 24 + p * 128;
        uint32_t word;
        for (int b = 0; b < 8; b++) g[4 + b] = 0x00;    /* shift0/filter0 */
        if (stereo) {
            uint32_t l = (uint32_t)(nib_l & 0xF), r = (uint32_t)(nib_r & 0xF);
            word = l | (r << 4) | (l << 8) | (r << 12)
                 | (l << 16) | (r << 20) | (l << 24) | (r << 28);
        } else {
            uint32_t n = (uint32_t)(nib_all & 0xF);
            word = n | (n << 4) | (n << 8) | (n << 12)
                 | (n << 16) | (n << 20) | (n << 24) | (n << 28);
        }
        for (int w = 0; w < 28; w++) {
            g[16 + w * 4 + 0] = (uint8_t)(word & 0xFFu);
            g[16 + w * 4 + 1] = (uint8_t)((word >> 8) & 0xFFu);
            g[16 + w * 4 + 2] = (uint8_t)((word >> 16) & 0xFFu);
            g[16 + w * 4 + 3] = (uint8_t)((word >> 24) & 0xFFu);
        }
    }
}

/* 8-bit sector: parameter bytes at +4, data bytes at +16 (word byte N =
 * sub-block N).  Stereo: sub-blocks 0,2 = L, 1,3 = R. */
static void xa_build_8bit_sector(uint8_t *sector, int stereo, int8_t l, int8_t r)
{
    memset(sector, 0, 2352);
    sector[18] = 0x64;
    sector[19] = (uint8_t)(0x04u | (stereo ? 1u : 0u));
    for (int p = 0; p < 18; p++) {
        uint8_t *g = sector + 24 + p * 128;
        for (int b = 0; b < 4; b++) g[4 + b] = 0x00;
        for (int w = 0; w < 28; w++) {
            g[16 + w * 4 + 0] = (uint8_t)(stereo ? l : l);
            g[16 + w * 4 + 1] = (uint8_t)(stereo ? r : l);
            g[16 + w * 4 + 2] = (uint8_t)(stereo ? l : l);
            g[16 + w * 4 + 3] = (uint8_t)(stereo ? r : l);
        }
    }
}

static unsigned xa_mix_count_nonzero(int16_t *buf, unsigned frames)
{
    unsigned nz = 0;
    memset(buf, 0, sizeof(int16_t) * frames * 2u);
    PE_Xa_Mix(buf, frames);
    for (unsigned i = 0; i < frames; i++)
        if (buf[i * 2u] != 0 || buf[i * 2u + 1u] != 0) nz++;
    return nz;
}

static void xa_mix_peaks(unsigned frames, int *pl, int *pr)
{
    static int16_t buf[4096 * 2];
    *pl = *pr = 0;
    memset(buf, 0, sizeof(int16_t) * frames * 2u);
    PE_Xa_Mix(buf, frames);
    for (unsigned i = 0; i < frames; i++) {
        int l = buf[i * 2u] < 0 ? -buf[i * 2u] : buf[i * 2u];
        int r = buf[i * 2u + 1u] < 0 ? -buf[i * 2u + 1u] : buf[i * 2u + 1u];
        if (l > *pl) *pl = l;
        if (r > *pr) *pr = r;
    }
}

/* ── 4-bit ADPCM decode ─────────────────────────────────────────────────── */
static void test_XA_adpcm_4bit_basic(void)
{
    static const int blocks[1] = { 0 };
    uint8_t group[128];
    int16_t out[8 * 28];
    int prev[2] = { 0, 0 };
    unsigned n;

    TEST("XA_adpcm_4bit_basic");
    memset(group, 0, sizeof group);
    group[0x10] = 0x01;                 /* block 0, sample 0 nibble = 1 */
    n = PE_Xa_DecodeGroup4(group, blocks, 1u, prev, out);
    ASSERT(n == 28u, "4-bit group must decode 28 samples");
    ASSERT(out[0] == 0x1000, "nibble 1 with shift 0 must decode to 0x1000");
    ASSERT(out[1] == 0 && out[27] == 0, "zero nibbles stay zero with filter 0");
    PASS();
}

static void test_XA_adpcm_4bit_filter(void)
{
    static const int blocks[1] = { 0 };
    uint8_t group[128];
    int16_t out[8 * 28];
    int prev[2] = { 0, 0 };

    TEST("XA_adpcm_4bit_filter");
    memset(group, 0, sizeof group);
    group[4] = 0x10;                    /* filter = 1 (60 / 0) */
    group[0x10] = 0x01;                 /* sample 0 = 1, sample 1 = 0 */
    PE_Xa_DecodeGroup4(group, blocks, 1u, prev, out);
    ASSERT(out[0] == 0x1000, "filter-1 first sample");
    /* 4096*60/64 truncated = 3840 */
    ASSERT(out[1] == 3840, "filter-1 prediction of the second sample");
    PASS();
}

static void test_XA_adpcm_4bit_signed_shift(void)
{
    static const int blocks[1] = { 0 };
    uint8_t group[128];
    int16_t out[8 * 28];
    int prev[2] = { 0, 0 };

    TEST("XA_adpcm_4bit_signed_shift");
    memset(group, 0, sizeof group);
    group[0x10] = 0x0F;                 /* nibble 0xF == -1 */
    PE_Xa_DecodeGroup4(group, blocks, 1u, prev, out);
    ASSERT(out[0] == -4096, "nibble 0xF must sign-extend to -4096");

    memset(group, 0, sizeof group);
    group[4] = 0x01;                    /* shift = 1 */
    group[0x10] = 0x01;
    prev[0] = prev[1] = 0;
    PE_Xa_DecodeGroup4(group, blocks, 1u, prev, out);
    ASSERT(out[0] == 2048, "shift 1 halves the sample");
    PASS();
}

/* ── 8-bit ADPCM decode (corrected layout) ──────────────────────────────── */
static void test_XA_adpcm_8bit_basic(void)
{
    static const int blocks[1] = { 0 };
    uint8_t group[128];
    int16_t out[4 * 28];
    int prev[2] = { 0, 0 };

    TEST("XA_adpcm_8bit_basic");
    memset(group, 0, sizeof group);
    group[16] = 0x01;                   /* word 0, byte 0 == sub-block 0 */
    PE_Xa_DecodeGroup8(group, blocks, 1u, prev, out);
    ASSERT(out[0] == 256, "8-bit sample 1 with shift 0 is 0x0100");
    memset(group, 0, sizeof group);
    group[16] = 0xFF;                   /* -1 */
    prev[0] = prev[1] = 0;
    PE_Xa_DecodeGroup8(group, blocks, 1u, prev, out);
    ASSERT(out[0] == -256, "8-bit sample -1 is -0x0100");
    PASS();
}

static void test_XA_adpcm_8bit_layout(void)
{
    static const int b0[1] = { 0 };
    static const int b1[1] = { 1 };
    uint8_t group[128];
    int16_t out[4 * 28];
    int prev[2] = { 0, 0 };

    TEST("XA_adpcm_8bit_layout");
    /* Data lives at +16; the +4..15 header bytes are not sample data. */
    memset(group, 0, sizeof group);
    group[16] = 0x02;                   /* sub-block 0, word 0 */
    PE_Xa_DecodeGroup8(group, b0, 1u, prev, out);
    ASSERT(out[0] == 512, "sub-block 0 sample comes from word byte 0 at +16");

    memset(group, 0, sizeof group);
    group[8] = 0x7F;                    /* the pre-change code read this as data */
    prev[0] = prev[1] = 0;
    PE_Xa_DecodeGroup8(group, b0, 1u, prev, out);
    ASSERT(out[0] == 0, "8-bit decode must not read sample data from +8");

    memset(group, 0, sizeof group);
    group[16 + 1] = 0x03;               /* word 0, byte 1 == sub-block 1 */
    prev[0] = prev[1] = 0;
    PE_Xa_DecodeGroup8(group, b1, 1u, prev, out);
    ASSERT(out[0] == 768, "sub-block 1 sample comes from word byte 1");
    PASS();
}

/* ── gaussian interpolation (psx-spx 512-entry table) ───────────────────── */
static void test_XA_gauss_taps(void)
{
    const int A = 32767;

    TEST("XA_gauss_taps");
    /* interp(oldest..newest, i): isolated taps equal (gauss[k]*A)>>15 with the
     * psx-spx entries gauss[0FFh]=12C7h, gauss[1FFh]=59B3h, gauss[100h]=1307h,
     * gauss[0]=FFFFh. */
    ASSERT(PE_Xa_GaussInterp(A, 0, 0, 0, 0x00u) == 4806, "oldest tap = (12C7h*A)>>15");
    ASSERT(PE_Xa_GaussInterp(0, A, 0, 0, 0x00u) == 22962, "older tap = (59B3h*A)>>15");
    ASSERT(PE_Xa_GaussInterp(0, 0, A, 0, 0x00u) == 4870, "old tap = (1307h*A)>>15");
    ASSERT(PE_Xa_GaussInterp(0, 0, 0, A, 0x00u) == -1, "newest tap = (FFFFh*A)>>15");
    /* Mid phase i=80h uses gauss[07Fh]=019Ch, gauss[17Fh]=3DEFh,
     * gauss[180h]=3E4Ch, gauss[080h]=01A8h. */
    ASSERT(PE_Xa_GaussInterp(A, 0, 0, 0, 0x80u) == 411, "oldest tap at i=80h");
    ASSERT(PE_Xa_GaussInterp(0, A, 0, 0, 0x80u) == 15854, "older tap at i=80h");
    ASSERT(PE_Xa_GaussInterp(0, 0, A, 0, 0x80u) == 15947, "old tap at i=80h");
    ASSERT(PE_Xa_GaussInterp(0, 0, 0, A, 0x80u) == 423, "newest tap at i=80h");
    PASS();
}

static void test_XA_gauss_normalization(void)
{
    int const A = 20000;

    TEST("XA_gauss_normalization");
    /* The four taps of every phase sum to 7F80h +/- 1 (psx-spx: the table is
     * normalised to 255/256), so a constant input settles to A*7F80h>>15 with
     * at most 1 LSB of spread and never overshoots. */
    for (unsigned i = 0; i < 256u; i++) {
        int16_t v = PE_Xa_GaussInterp(A, A, A, A, i);
        ASSERT(v >= 19921 && v <= 19922, "constant input stays at A*7F80h>>15");
        ASSERT(v <= A, "gaussian never overshoots a constant input");
    }
    PASS();
}

static void test_XA_gauss_step(void)
{
    const int A = 32767;
    int prev_v = -2;

    TEST("XA_gauss_step");
    /* Step arriving at the newest sample: the tap weight gauss[000h+i] rises
     * monotonically with the phase, so the step response is a monotone ramp
     * from silence to the first partial value (no ringing). */
    for (unsigned i = 0; i < 256u; i++) {
        int v = PE_Xa_GaussInterp(0, 0, 0, A, i);
        ASSERT(v >= prev_v, "gaussian step response is monotone");
        prev_v = v;
    }
    ASSERT(prev_v == 4806, "newest-sample step ends at gauss[0FFh]*A>>15");
    /* A step at the `older` tap starts near 0.85*A and settles at ~A. */
    ASSERT(PE_Xa_GaussInterp(0, A, A, A, 0x00u) == 27832, "older-tap step at i=0");
    ASSERT(PE_Xa_GaussInterp(0, A, A, A, 0xFFu) == 32640, "older-tap step settles at i=FFh");
    PASS();
}

/* ── sector classification / resample ───────────────────────────────────── */
static void test_XA_sector_classification(void)
{
    uint8_t sector[2352];

    TEST("XA_sector_classification");
    PE_Xa_Reset();
    memset(sector, 0, sizeof sector);
    ASSERT(PE_Xa_ConsumeSector(sector) == 0u,
           "a zero-submode sector is not XA audio");
    /* Form-2 XA audio: submode bit 2, coding info stereo / 37800 / 4-bit. */
    xa_build_4bit_sector(sector, 1, 0, 0, 7, 1);
    ASSERT(PE_Xa_ConsumeSector(sector) > 0u,
           "an audio sector must decode samples");
    ASSERT(PE_Xa_SectorsDecoded() == 1u, "one audio sector counted");
    ASSERT(PE_Xa_ActiveChannels() == 2u, "stereo coding info recorded");
    PASS();
}

static void test_XA_mix_resample(void)
{
    uint8_t sector[2352];
    static int16_t buf[64 * 2];
    int peak = 0;

    TEST("XA_mix_resample");
    PE_Xa_Reset();
    xa_audio_path_default();
    xa_build_4bit_sector(sector, 1, 0, 0, 7, 1);
    ASSERT(PE_Xa_ConsumeSector(sector) > 0u, "sector decodes");
    memset(buf, 0, sizeof buf);
    PE_Xa_Mix(buf, 64u);
    for (unsigned i = 0; i < 64u; i++) {
        int v = buf[i] < 0 ? -buf[i] : buf[i];
        if (v > peak) peak = v;
    }
    ASSERT(peak > 0, "resampled XA output must be non-silent");
    PASS();
}

/* 37800 Hz / 4-bit / mono: 8 sub-blocks * 28 = 224 frames per group *
 * 18 groups = 4032 source frames; at step 6/7 that is ~4702 sink frames. */
static void test_XA_resample_37800_mono(void)
{
    uint8_t sector[2352];
    static int16_t buf[8192 * 2];
    unsigned nz;

    TEST("XA_resample_37800_mono");
    PE_Xa_Reset();
    xa_audio_path_default();
    xa_build_4bit_sector(sector, 0, 0, 5, 0, 0);
    ASSERT(PE_Xa_ConsumeSector(sector) == 4032u, "4-bit mono sector decodes 4032 source frames");
    ASSERT(PE_Xa_ActiveChannels() == 1u, "mono coding info recorded");
    nz = xa_mix_count_nonzero(buf, 8192u);
    ASSERT(nz >= 4690u && nz <= 4715u, "37800 mono resamples to ~4702 output frames");
    ASSERT(buf[100] == buf[101], "mono XA reaches both output channels");
    PASS();
}

/* 18900 Hz / 4-bit / mono: 4032 source frames at step 3/7 => ~9404 sink
 * frames.  This is the half-rate path that was never exercised before. */
static void test_XA_resample_18900_mono(void)
{
    uint8_t sector[2352];
    static int16_t buf[12000 * 2];
    unsigned nz;

    TEST("XA_resample_18900_mono");
    PE_Xa_Reset();
    xa_audio_path_default();
    xa_build_4bit_sector(sector, 0, 1, 5, 0, 0);
    ASSERT(PE_Xa_ConsumeSector(sector) == 4032u, "18900 mono source frames");
    nz = xa_mix_count_nonzero(buf, 12000u);
    ASSERT(nz >= 9380u && nz <= 9420u, "18900 mono resamples to ~9404 output frames");
    ASSERT(buf[100] == buf[101], "18900 mono reaches both output channels");
    PASS();
}

/* 8-bit / stereo: 28 words * 4 bytes = 112 source frames per group *
 * 18 = 1008 per sector; the left/right byte split must survive. */
static void test_XA_resample_8bit_stereo(void)
{
    uint8_t sector[2352];
    static int16_t buf[2048 * 2];
    unsigned nz, i;
    int pl = 0, pr = 0;

    TEST("XA_resample_8bit_stereo");
    PE_Xa_Reset();
    xa_audio_path_default();
    xa_build_8bit_sector(sector, 1, 40, 5);
    ASSERT(PE_Xa_ConsumeSector(sector) == 1008u, "8-bit stereo sector decodes 1008 source frames");
    ASSERT(PE_Xa_ActiveChannels() == 2u, "8-bit stereo coding info recorded");
    nz = xa_mix_count_nonzero(buf, 2048u);
    ASSERT(nz >= 1160u && nz <= 1190u, "1008 source frames resample to ~1174 sink frames");
    for (i = 0; i < 2048u; i++) {
        int l = buf[i * 2u] < 0 ? -buf[i * 2u] : buf[i * 2u];
        int r = buf[i * 2u + 1u] < 0 ? -buf[i * 2u + 1u] : buf[i * 2u + 1u];
        if (l > pl) pl = l;
        if (r > pr) pr = r;
    }
    ASSERT(pl > 0 && pr > 0, "8-bit stereo decodes both channels");
    ASSERT(pl > pr, "8-bit left/right byte split is preserved");
    PASS();
}

/* ── volume chain: ATV0..3 + SPU CD volume + main volume + SPUCNT gate ──── */
static void test_XA_volume_matrix(void)
{
    uint8_t sector[2352];
    int pl, pr, ml, mr, zl, zr;

    TEST("XA_volume_matrix");
    xa_build_4bit_sector(sector, 1, 0, 0, 7, 1);

    /* Identity matrix 80h,0,80h,0: left stronger, both audible. */
    PE_Xa_Reset(); xa_audio_path_default();
    ASSERT(PE_Xa_ConsumeSector(sector) == 2016u, "4-bit stereo sector frames");
    xa_mix_peaks(3000u, &pl, &pr);
    ASSERT(pl > 0 && pr > 0 && pl > pr, "identity matrix: both channels, L stronger");

    /* Mono matrix 40h x4 folds L and R equally into both channels. */
    PE_Xa_Reset(); xa_audio_path_default(); xa_set_atv(0x40u, 0x40u, 0x40u, 0x40u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &ml, &mr);
    ASSERT(ml > 0, "mono matrix produces output");
    { int d = ml - mr; if (d < 0) d = -d; ASSERT(d <= 2, "mono matrix makes L == R"); }

    /* All-zero ATV0..3 silences the XA contribution. */
    PE_Xa_Reset(); xa_audio_path_default(); xa_set_atv(0u, 0u, 0u, 0u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl == 0 && zr == 0, "zero ATV matrix is silent");

    /* SPU CD audio input volume zero silences. */
    PE_Xa_Reset(); xa_audio_path_default();
    PE_SpuRegister_StoreU16(0x1B0u, 0u); PE_SpuRegister_StoreU16(0x1B2u, 0u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl == 0 && zr == 0, "zero CD input volume is silent");

    /* SPUCNT CD-audio-enable (bit0) gating. */
    PE_Xa_Reset(); xa_audio_path_default();
    PE_SpuRegister_StoreU16(0x1AAu, 0xC000u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl == 0 && zr == 0, "CD audio disabled (SPUCNT bit0=0) is silent");

    /* SPUCNT mute (bit14 clear) gates the whole mixer. */
    PE_Xa_Reset(); xa_audio_path_default();
    PE_SpuRegister_StoreU16(0x1AAu, 0x8001u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl == 0 && zr == 0, "muted SPU silences XA");

    /* FFh ATV (double volume) raises the left level above unity. */
    PE_Xa_Reset(); xa_audio_path_default(); xa_set_atv(0xFFu, 0u, 0xFFu, 0u);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl > pl, "FFh ATV (double volume) raises the left level");

    /* Half CD volume halves the contribution. */
    PE_Xa_Reset(); xa_audio_path_default();
    PE_SpuRegister_StoreU16(0x1B0u, 0x3FFFu >> 1); PE_SpuRegister_StoreU16(0x1B2u, 0x3FFFu >> 1);
    PE_Xa_ConsumeSector(sector);
    xa_mix_peaks(3000u, &zl, &zr);
    ASSERT(zl > pl / 2 - 2 && zl < pl / 2 + 2, "half CD volume halves the output");
    PASS();
}

static void test_XA_run_all(void)
{
    test_XA_adpcm_4bit_basic();
    test_XA_adpcm_4bit_filter();
    test_XA_adpcm_4bit_signed_shift();
    test_XA_adpcm_8bit_basic();
    test_XA_adpcm_8bit_layout();
    test_XA_gauss_taps();
    test_XA_gauss_normalization();
    test_XA_gauss_step();
    test_XA_sector_classification();
    test_XA_mix_resample();
    test_XA_resample_37800_mono();
    test_XA_resample_18900_mono();
    test_XA_resample_8bit_stereo();
    test_XA_volume_matrix();
}
