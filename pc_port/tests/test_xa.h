/*
 * test_xa.h — unit vectors for the CD-XA ADPCM decoder (pe_xa.c).
 *
 * The vectors are hand-computed from the decode algorithm (psx-spx filter
 * tables + the reference sound-group layout): a param byte of 0 means
 * shift=0/filter=0, so a lone nibble N decodes to (int16)(N<<12) exactly.
 */
#include "pe_xa.h"

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

static void test_XA_adpcm_8bit_basic(void)
{
    static const int blocks[1] = { 0 };
    uint8_t group[128];
    int16_t out[4 * 28];
    int prev[2] = { 0, 0 };

    TEST("XA_adpcm_8bit_basic");
    memset(group, 0, sizeof group);
    group[8] = 0x01;                    /* block 0 sample 0 = 1 */
    PE_Xa_DecodeGroup8(group, blocks, 1u, prev, out);
    ASSERT(out[0] == 256, "8-bit sample 1 with shift 0 is 0x0100");
    memset(group, 0, sizeof group);
    group[8] = 0xFF;                    /* -1 */
    prev[0] = prev[1] = 0;
    PE_Xa_DecodeGroup8(group, blocks, 1u, prev, out);
    ASSERT(out[0] == -256, "8-bit sample -1 is -0x0100");
    PASS();
}

static void test_XA_sector_classification(void)
{
    uint8_t sector[2352];

    TEST("XA_sector_classification");
    PE_Xa_Reset();
    memset(sector, 0, sizeof sector);
    ASSERT(PE_Xa_ConsumeSector(sector) == 0u,
           "a zero-submode sector is not XA audio");
    /* Form-2 XA audio: submode bit 2, coding info stereo / 37800 / 4-bit. */
    sector[18] = 0x64;
    sector[19] = 0x01;
    ASSERT(PE_Xa_ConsumeSector(sector) > 0u,
           "an audio sector must decode samples");
    ASSERT(PE_Xa_SectorsDecoded() == 1u, "one audio sector counted");
    ASSERT(PE_Xa_ActiveChannels() == 2u, "stereo coding info recorded");
    PASS();
}

static void test_XA_mix_resample(void)
{
    uint8_t sector[2352];
    int16_t buf[64];
    int peak = 0;

    TEST("XA_mix_resample");
    PE_Xa_Reset();
    memset(sector, 0, sizeof sector);
    sector[18] = 0x64;
    sector[19] = 0x01;                  /* stereo 37800 4-bit */
    sector[24 + 4] = 0x00;              /* packet 0 block 0 param */
    sector[24 + 0x10] = 0x07;           /* block 0 sample 0 = 7 */
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

static void test_XA_run_all(void)
{
    test_XA_adpcm_4bit_basic();
    test_XA_adpcm_4bit_filter();
    test_XA_adpcm_4bit_signed_shift();
    test_XA_adpcm_8bit_basic();
    test_XA_sector_classification();
    test_XA_mix_resample();
}
