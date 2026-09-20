/*
 * pe_xa.c — CD-XA ADPCM decoder + 44.1 kHz resampler for the native port.
 * See pe_xa.h for provenance and documented gaps.
 */
#include "pe_xa.h"
#include "pe_audio.h"
#include "pe_cdreg.h"
#include "pe_spu_dma.h"

#include <string.h>

#define XA_PACKETS       18
#define XA_GROUP_BYTES   128
#define XA_BLOCK_SAMPLES 28
#define XA_RING_FRAMES   32768u

/* SPU register offsets (relative to 0x1F801C00), psx-spx "SPU I/O Ports". */
#define SPU_MAIN_VOL_L   0x180u   /* 1F801D80h main volume left              */
#define SPU_MAIN_VOL_R   0x182u   /* 1F801D82h main volume right             */
#define SPU_SPUCNT       0x1AAu   /* 1F801DAAh control (enable/mute/CD enable)*/
#define SPU_CD_VOL_L     0x1B0u   /* 1F801DB0h CD audio input volume left    */
#define SPU_CD_VOL_R     0x1B2u   /* 1F801DB2h CD audio input volume right   */

/* psx-spx / reference-implementation XA prediction filter tables. */
static const int kFiltPos[5] = { 0, 60, 115, 98, 122 };
static const int kFiltNeg[5] = { 0, 0, -52, -55, -60 };

/* psx-spx "4-Point Gaussian Interpolation" table: 512 signed entries
 * (the first sixteen are -1, i.e. 0xFFFF).  Cross-checked against the
 * identical table in DuckStation src/core/spu.cpp GenerateInterpolationCoefficients(). */
static const int16_t kGauss[512] = {
    (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF, (int16_t)0xFFFF,
    (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0000, (int16_t)0x0001, (int16_t)0x0001, (int16_t)0x0001, (int16_t)0x0001, (int16_t)0x0002, (int16_t)0x0002, (int16_t)0x0002, (int16_t)0x0003, (int16_t)0x0003,
    (int16_t)0x0003, (int16_t)0x0004, (int16_t)0x0004, (int16_t)0x0005, (int16_t)0x0005, (int16_t)0x0006, (int16_t)0x0007, (int16_t)0x0007, (int16_t)0x0008, (int16_t)0x0009, (int16_t)0x0009, (int16_t)0x000A, (int16_t)0x000B, (int16_t)0x000C, (int16_t)0x000D, (int16_t)0x000E,
    (int16_t)0x000F, (int16_t)0x0010, (int16_t)0x0011, (int16_t)0x0012, (int16_t)0x0013, (int16_t)0x0015, (int16_t)0x0016, (int16_t)0x0018, (int16_t)0x0019, (int16_t)0x001B, (int16_t)0x001C, (int16_t)0x001E, (int16_t)0x0020, (int16_t)0x0021, (int16_t)0x0023, (int16_t)0x0025,
    (int16_t)0x0027, (int16_t)0x0029, (int16_t)0x002C, (int16_t)0x002E, (int16_t)0x0030, (int16_t)0x0033, (int16_t)0x0035, (int16_t)0x0038, (int16_t)0x003A, (int16_t)0x003D, (int16_t)0x0040, (int16_t)0x0043, (int16_t)0x0046, (int16_t)0x0049, (int16_t)0x004D, (int16_t)0x0050,
    (int16_t)0x0054, (int16_t)0x0057, (int16_t)0x005B, (int16_t)0x005F, (int16_t)0x0063, (int16_t)0x0067, (int16_t)0x006B, (int16_t)0x006F, (int16_t)0x0074, (int16_t)0x0078, (int16_t)0x007D, (int16_t)0x0082, (int16_t)0x0087, (int16_t)0x008C, (int16_t)0x0091, (int16_t)0x0096,
    (int16_t)0x009C, (int16_t)0x00A1, (int16_t)0x00A7, (int16_t)0x00AD, (int16_t)0x00B3, (int16_t)0x00BA, (int16_t)0x00C0, (int16_t)0x00C7, (int16_t)0x00CD, (int16_t)0x00D4, (int16_t)0x00DB, (int16_t)0x00E3, (int16_t)0x00EA, (int16_t)0x00F2, (int16_t)0x00FA, (int16_t)0x0101,
    (int16_t)0x010A, (int16_t)0x0112, (int16_t)0x011B, (int16_t)0x0123, (int16_t)0x012C, (int16_t)0x0135, (int16_t)0x013F, (int16_t)0x0148, (int16_t)0x0152, (int16_t)0x015C, (int16_t)0x0166, (int16_t)0x0171, (int16_t)0x017B, (int16_t)0x0186, (int16_t)0x0191, (int16_t)0x019C,
    (int16_t)0x01A8, (int16_t)0x01B4, (int16_t)0x01C0, (int16_t)0x01CC, (int16_t)0x01D9, (int16_t)0x01E5, (int16_t)0x01F2, (int16_t)0x0200, (int16_t)0x020D, (int16_t)0x021B, (int16_t)0x0229, (int16_t)0x0237, (int16_t)0x0246, (int16_t)0x0255, (int16_t)0x0264, (int16_t)0x0273,
    (int16_t)0x0283, (int16_t)0x0293, (int16_t)0x02A3, (int16_t)0x02B4, (int16_t)0x02C4, (int16_t)0x02D6, (int16_t)0x02E7, (int16_t)0x02F9, (int16_t)0x030B, (int16_t)0x031D, (int16_t)0x0330, (int16_t)0x0343, (int16_t)0x0356, (int16_t)0x036A, (int16_t)0x037E, (int16_t)0x0392,
    (int16_t)0x03A7, (int16_t)0x03BC, (int16_t)0x03D1, (int16_t)0x03E7, (int16_t)0x03FC, (int16_t)0x0413, (int16_t)0x042A, (int16_t)0x0441, (int16_t)0x0458, (int16_t)0x0470, (int16_t)0x0488, (int16_t)0x04A0, (int16_t)0x04B9, (int16_t)0x04D2, (int16_t)0x04EC, (int16_t)0x0506,
    (int16_t)0x0520, (int16_t)0x053B, (int16_t)0x0556, (int16_t)0x0572, (int16_t)0x058E, (int16_t)0x05AA, (int16_t)0x05C7, (int16_t)0x05E4, (int16_t)0x0601, (int16_t)0x061F, (int16_t)0x063E, (int16_t)0x065C, (int16_t)0x067C, (int16_t)0x069B, (int16_t)0x06BB, (int16_t)0x06DC,
    (int16_t)0x06FD, (int16_t)0x071E, (int16_t)0x0740, (int16_t)0x0762, (int16_t)0x0784, (int16_t)0x07A7, (int16_t)0x07CB, (int16_t)0x07EF, (int16_t)0x0813, (int16_t)0x0838, (int16_t)0x085D, (int16_t)0x0883, (int16_t)0x08A9, (int16_t)0x08D0, (int16_t)0x08F7, (int16_t)0x091E,
    (int16_t)0x0946, (int16_t)0x096F, (int16_t)0x0998, (int16_t)0x09C1, (int16_t)0x09EB, (int16_t)0x0A16, (int16_t)0x0A40, (int16_t)0x0A6C, (int16_t)0x0A98, (int16_t)0x0AC4, (int16_t)0x0AF1, (int16_t)0x0B1E, (int16_t)0x0B4C, (int16_t)0x0B7A, (int16_t)0x0BA9, (int16_t)0x0BD8,
    (int16_t)0x0C07, (int16_t)0x0C38, (int16_t)0x0C68, (int16_t)0x0C99, (int16_t)0x0CCB, (int16_t)0x0CFD, (int16_t)0x0D30, (int16_t)0x0D63, (int16_t)0x0D97, (int16_t)0x0DCB, (int16_t)0x0E00, (int16_t)0x0E35, (int16_t)0x0E6B, (int16_t)0x0EA1, (int16_t)0x0ED7, (int16_t)0x0F0F,
    (int16_t)0x0F46, (int16_t)0x0F7F, (int16_t)0x0FB7, (int16_t)0x0FF1, (int16_t)0x102A, (int16_t)0x1065, (int16_t)0x109F, (int16_t)0x10DB, (int16_t)0x1116, (int16_t)0x1153, (int16_t)0x118F, (int16_t)0x11CD, (int16_t)0x120B, (int16_t)0x1249, (int16_t)0x1288, (int16_t)0x12C7,
    (int16_t)0x1307, (int16_t)0x1347, (int16_t)0x1388, (int16_t)0x13C9, (int16_t)0x140B, (int16_t)0x144D, (int16_t)0x1490, (int16_t)0x14D4, (int16_t)0x1517, (int16_t)0x155C, (int16_t)0x15A0, (int16_t)0x15E6, (int16_t)0x162C, (int16_t)0x1672, (int16_t)0x16B9, (int16_t)0x1700,
    (int16_t)0x1747, (int16_t)0x1790, (int16_t)0x17D8, (int16_t)0x1821, (int16_t)0x186B, (int16_t)0x18B5, (int16_t)0x1900, (int16_t)0x194B, (int16_t)0x1996, (int16_t)0x19E2, (int16_t)0x1A2E, (int16_t)0x1A7B, (int16_t)0x1AC8, (int16_t)0x1B16, (int16_t)0x1B64, (int16_t)0x1BB3,
    (int16_t)0x1C02, (int16_t)0x1C51, (int16_t)0x1CA1, (int16_t)0x1CF1, (int16_t)0x1D42, (int16_t)0x1D93, (int16_t)0x1DE5, (int16_t)0x1E37, (int16_t)0x1E89, (int16_t)0x1EDC, (int16_t)0x1F2F, (int16_t)0x1F82, (int16_t)0x1FD6, (int16_t)0x202A, (int16_t)0x207F, (int16_t)0x20D4,
    (int16_t)0x2129, (int16_t)0x217F, (int16_t)0x21D5, (int16_t)0x222C, (int16_t)0x2282, (int16_t)0x22DA, (int16_t)0x2331, (int16_t)0x2389, (int16_t)0x23E1, (int16_t)0x2439, (int16_t)0x2492, (int16_t)0x24EB, (int16_t)0x2545, (int16_t)0x259E, (int16_t)0x25F8, (int16_t)0x2653,
    (int16_t)0x26AD, (int16_t)0x2708, (int16_t)0x2763, (int16_t)0x27BE, (int16_t)0x281A, (int16_t)0x2876, (int16_t)0x28D2, (int16_t)0x292E, (int16_t)0x298B, (int16_t)0x29E7, (int16_t)0x2A44, (int16_t)0x2AA1, (int16_t)0x2AFF, (int16_t)0x2B5C, (int16_t)0x2BBA, (int16_t)0x2C18,
    (int16_t)0x2C76, (int16_t)0x2CD4, (int16_t)0x2D33, (int16_t)0x2D91, (int16_t)0x2DF0, (int16_t)0x2E4F, (int16_t)0x2EAE, (int16_t)0x2F0D, (int16_t)0x2F6C, (int16_t)0x2FCC, (int16_t)0x302B, (int16_t)0x308B, (int16_t)0x30EA, (int16_t)0x314A, (int16_t)0x31AA, (int16_t)0x3209,
    (int16_t)0x3269, (int16_t)0x32C9, (int16_t)0x3329, (int16_t)0x3389, (int16_t)0x33E9, (int16_t)0x3449, (int16_t)0x34A9, (int16_t)0x3509, (int16_t)0x3569, (int16_t)0x35C9, (int16_t)0x3629, (int16_t)0x3689, (int16_t)0x36E8, (int16_t)0x3748, (int16_t)0x37A8, (int16_t)0x3807,
    (int16_t)0x3867, (int16_t)0x38C6, (int16_t)0x3926, (int16_t)0x3985, (int16_t)0x39E4, (int16_t)0x3A43, (int16_t)0x3AA2, (int16_t)0x3B00, (int16_t)0x3B5F, (int16_t)0x3BBD, (int16_t)0x3C1B, (int16_t)0x3C79, (int16_t)0x3CD7, (int16_t)0x3D35, (int16_t)0x3D92, (int16_t)0x3DEF,
    (int16_t)0x3E4C, (int16_t)0x3EA9, (int16_t)0x3F05, (int16_t)0x3F62, (int16_t)0x3FBD, (int16_t)0x4019, (int16_t)0x4074, (int16_t)0x40D0, (int16_t)0x412A, (int16_t)0x4185, (int16_t)0x41DF, (int16_t)0x4239, (int16_t)0x4292, (int16_t)0x42EB, (int16_t)0x4344, (int16_t)0x439C,
    (int16_t)0x43F4, (int16_t)0x444C, (int16_t)0x44A3, (int16_t)0x44FA, (int16_t)0x4550, (int16_t)0x45A6, (int16_t)0x45FC, (int16_t)0x4651, (int16_t)0x46A6, (int16_t)0x46FA, (int16_t)0x474E, (int16_t)0x47A1, (int16_t)0x47F4, (int16_t)0x4846, (int16_t)0x4898, (int16_t)0x48E9,
    (int16_t)0x493A, (int16_t)0x498A, (int16_t)0x49D9, (int16_t)0x4A29, (int16_t)0x4A77, (int16_t)0x4AC5, (int16_t)0x4B13, (int16_t)0x4B5F, (int16_t)0x4BAC, (int16_t)0x4BF7, (int16_t)0x4C42, (int16_t)0x4C8D, (int16_t)0x4CD7, (int16_t)0x4D20, (int16_t)0x4D68, (int16_t)0x4DB0,
    (int16_t)0x4DF7, (int16_t)0x4E3E, (int16_t)0x4E84, (int16_t)0x4EC9, (int16_t)0x4F0E, (int16_t)0x4F52, (int16_t)0x4F95, (int16_t)0x4FD7, (int16_t)0x5019, (int16_t)0x505A, (int16_t)0x509A, (int16_t)0x50DA, (int16_t)0x5118, (int16_t)0x5156, (int16_t)0x5194, (int16_t)0x51D0,
    (int16_t)0x520C, (int16_t)0x5247, (int16_t)0x5281, (int16_t)0x52BA, (int16_t)0x52F3, (int16_t)0x532A, (int16_t)0x5361, (int16_t)0x5397, (int16_t)0x53CC, (int16_t)0x5401, (int16_t)0x5434, (int16_t)0x5467, (int16_t)0x5499, (int16_t)0x54CA, (int16_t)0x54FA, (int16_t)0x5529,
    (int16_t)0x5558, (int16_t)0x5585, (int16_t)0x55B2, (int16_t)0x55DE, (int16_t)0x5609, (int16_t)0x5632, (int16_t)0x565B, (int16_t)0x5684, (int16_t)0x56AB, (int16_t)0x56D1, (int16_t)0x56F6, (int16_t)0x571B, (int16_t)0x573E, (int16_t)0x5761, (int16_t)0x5782, (int16_t)0x57A3,
    (int16_t)0x57C3, (int16_t)0x57E2, (int16_t)0x57FF, (int16_t)0x581C, (int16_t)0x5838, (int16_t)0x5853, (int16_t)0x586D, (int16_t)0x5886, (int16_t)0x589E, (int16_t)0x58B5, (int16_t)0x58CB, (int16_t)0x58E0, (int16_t)0x58F4, (int16_t)0x5907, (int16_t)0x5919, (int16_t)0x592A,
    (int16_t)0x593A, (int16_t)0x5949, (int16_t)0x5958, (int16_t)0x5965, (int16_t)0x5971, (int16_t)0x597C, (int16_t)0x5986, (int16_t)0x598F, (int16_t)0x5997, (int16_t)0x599E, (int16_t)0x59A4, (int16_t)0x59A9, (int16_t)0x59AD, (int16_t)0x59B0, (int16_t)0x59B2, (int16_t)0x59B3,
};

static int16_t clamp16(int32_t v)
{
    if (v > 0x7FFF) return 0x7FFF;
    if (v < -0x8000) return (int16_t)-0x8000;
    return (int16_t)v;
}

static int32_t s_prev_l[2], s_prev_r[2];
static int16_t s_ring[XA_RING_FRAMES * 2u];
static unsigned long long s_head, s_tail;   /* ring frame indices */
static unsigned s_src_rate = 37800u;
static unsigned s_phase;                    /* resampler phase numerator, < 44100 */
static unsigned long long s_sectors, s_frames;
static unsigned s_channels;

void PE_Xa_Reset(void)
{
    memset(s_prev_l, 0, sizeof s_prev_l);
    memset(s_prev_r, 0, sizeof s_prev_r);
    memset(s_ring, 0, sizeof s_ring);
    s_head = s_tail = 0u;
    s_src_rate = 37800u;
    s_phase = 0u;
    s_sectors = s_frames = 0u;
    s_channels = 0u;
}

/* ── PS1 4-point gaussian interpolation (psx-spx SPU chapter) ─────────────
 * out = (gauss[0FFh-i]*oldest + gauss[1FFh-i]*older + gauss[100h+i]*old
 *        + gauss[000h+i]*newest) SAR 15, i = 00h..FFh.
 * The four arguments are the four consecutive input samples oldest..newest,
 * exactly as psx-spx names them; the same table and ordering are used by the
 * DuckStation SPU voice interpolator. */
int16_t PE_Xa_GaussInterp(int oldest, int older, int old, int newest, unsigned index)
{
    unsigned i = index & 0xFFu;
    int32_t acc = (int32_t)kGauss[0xFFu - i] * oldest
                + (int32_t)kGauss[0x1FFu - i] * older
                + (int32_t)kGauss[0x100u + i] * old
                + (int32_t)kGauss[i] * newest;
    return clamp16(acc >> 15);
}

static void ring_push(int16_t l, int16_t r)
{
    /* Keep four frames of history headroom so the gaussian's oldest tap
     * (s_tail-1) is never the slot being overwritten. */
    if (s_head - s_tail >= XA_RING_FRAMES - 4u)
        s_tail++;                            /* drop oldest when saturated */
    unsigned i = (unsigned)(s_head % XA_RING_FRAMES);
    s_ring[i * 2u] = l;
    s_ring[i * 2u + 1u] = r;
    s_head++;
}

static int16_t ring_at(unsigned long long idx, unsigned ch)
{
    unsigned i = (unsigned)(idx % XA_RING_FRAMES);
    return s_ring[i * 2u + ch];
}

static void group_params(uint8_t param, int *shift, int *filter)
{
    int sh = param & 0x0Fu;
    int fl = (param & 0x30u) >> 4u;
    if (sh > 12) sh = 9;                     /* hardware behaviour */
    if (fl > 4) fl = 4;
    *shift = sh;
    *filter = fl;
}

unsigned PE_Xa_DecodeGroup4(const uint8_t group[128], const int *blocks,
                            unsigned block_count, int prev[2], int16_t *out)
{
    unsigned n = 0;
    for (unsigned b = 0; b < block_count; b++) {
        int blk = blocks[b];
        int shift, filter;
        int fp, fn;
        group_params(group[4 + blk], &shift, &filter);
        fp = kFiltPos[filter];
        fn = kFiltNeg[filter];
        for (int i = 0; i < XA_BLOCK_SAMPLES; i++) {
            uint32_t w = (uint32_t)group[0x10 + i * 4]
                       | ((uint32_t)group[0x10 + i * 4 + 1] << 8)
                       | ((uint32_t)group[0x10 + i * 4 + 2] << 16)
                       | ((uint32_t)group[0x10 + i * 4 + 3] << 24);
            int nib = (int)((w >> (blk * 4)) & 0xFu);
            int32_t s = (int32_t)(int16_t)(nib << 12);
            s >>= shift;
            s += (prev[0] * fp + prev[1] * fn + 32) / 64;
            out[n++] = clamp16(s);
            prev[1] = prev[0];
            prev[0] = s;
        }
    }
    return n;
}

unsigned PE_Xa_DecodeGroup8(const uint8_t group[128], const int *blocks,
                            unsigned block_count, int prev[2], int16_t *out)
{
    unsigned n = 0;
    for (unsigned b = 0; b < block_count; b++) {
        int blk = blocks[b];
        int shift, filter;
        int fp, fn;
        group_params(group[4 + blk], &shift, &filter);
        fp = kFiltPos[filter];
        fn = kFiltNeg[filter];
        for (int i = 0; i < XA_BLOCK_SAMPLES; i++) {
            /* psx-spx sound-group layout: 28 little-endian 32-bit words at
             * +16; byte `blk` of each word is sub-block `blk`'s sample. */
            int8_t raw = (int8_t)group[16 + i * 4 + blk];
            int32_t s = (int32_t)raw << 8;
            s >>= shift;
            s += (prev[0] * fp + prev[1] * fn + 32) / 64;
            out[n++] = clamp16(s);
            prev[1] = prev[0];
            prev[0] = s;
        }
    }
    return n;
}

unsigned PE_Xa_ConsumeSector(const uint8_t raw[2352])
{
    static const int mono4[8]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
    static const int left4[4]  = { 0, 2, 4, 6 };
    static const int right4[4] = { 1, 3, 5, 7 };
    static const int left8[2]  = { 0, 2 };
    static const int right8[2] = { 1, 3 };
    static const int mono8[4]  = { 0, 1, 2, 3 };
    const uint8_t *data;
    uint8_t submode, ci;
    int stereo, bits8;
    unsigned added = 0;

    if (!raw)
        return 0u;
    submode = raw[18];                       /* Form-2 subheader submode */
    if ((submode & 0x04u) == 0u)
        return 0u;                           /* not XA audio */
    ci = raw[19];                            /* coding info */
    stereo = (ci & 0x01u) != 0u;
    bits8  = (ci & 0x04u) != 0u;
    s_src_rate = (ci & 0x02u) ? 18900u : 37800u;
    s_channels = stereo ? 2u : 1u;
    data = raw + 24;                         /* Form-2 payload */

    for (int p = 0; p < XA_PACKETS; p++) {
        const uint8_t *g = data + p * XA_GROUP_BYTES;
        int16_t l[8 * XA_BLOCK_SAMPLES], r[8 * XA_BLOCK_SAMPLES];
        unsigned nl, nr, m, i;
        if (bits8) {
            if (stereo) {
                nl = PE_Xa_DecodeGroup8(g, left8, 2u, s_prev_l, l);
                nr = PE_Xa_DecodeGroup8(g, right8, 2u, s_prev_r, r);
            } else {
                nl = PE_Xa_DecodeGroup8(g, mono8, 4u, s_prev_l, l);
                nr = nl; memcpy(r, l, sizeof(int16_t) * nl);
            }
        } else {
            if (stereo) {
                nl = PE_Xa_DecodeGroup4(g, left4, 4u, s_prev_l, l);
                nr = PE_Xa_DecodeGroup4(g, right4, 4u, s_prev_r, r);
            } else {
                nl = PE_Xa_DecodeGroup4(g, mono4, 8u, s_prev_l, l);
                nr = nl; memcpy(r, l, sizeof(int16_t) * nl);
            }
        }
        m = nl < nr ? nl : nr;
        for (i = 0; i < m; i++)
            ring_push(l[i], r[i]);
        added += m;
    }
    s_sectors++;
    s_frames += added;
    return added;
}

/* ── CD-XA output volume chain ────────────────────────────────────────────
 * The decoded XA samples are scaled on the way to the sink by, in order:
 *   1. the CD controller ATV0..3 matrix (psx-spx "0x1f801802 (write, bank 2)
 *      ATV0 (L->L) / ATV1 (L->R) / ATV2 (R->R) / ATV3 (R->L)", 80h = unity,
 *      FFh = double, applying saturation),
 *   2. the SPU CD audio input volume (1F801DB0h/1F801DB2h, signed 16-bit,
 *      7FFFh = unity),
 *   3. the SPU main volume (1F801D80h/1F801D82h, fixed mode: bits0-14 are
 *      volume/2) — already applied to the voice mix by pe_spu, so applying it
 *      here makes the composite main*(voices + cd_gain*xa) exact.
 * SPUCNT bit0 (CD audio enable) and bit14 (mute) gate the contribution. */
typedef struct XaGain {
    int enabled;
    unsigned atv[4];
    int cd_l, cd_r;
    int mv_l, mv_r;
} XaGain;

/* psx-spx fixed-volume register decode (mirrors pe_spu.c voice_volume_gain). */
static int spu_fixed_volume(uint16_t reg)
{
    if (reg & 0x8000u) return 0;             /* sweep mode not swept (gap) */
    return (int)(int16_t)(uint16_t)(reg << 1);
}

static void xa_resolve_gain(XaGain *g)
{
    uint8_t atv[4];
    uint16_t spucnt = PE_SpuRegister_LoadU16(SPU_SPUCNT);

    g->enabled = ((spucnt & 0x4001u) == 0x4001u);   /* unmute + CD audio enable */
    PE_CdReg_GetAudioVolumes(atv);
    for (int i = 0; i < 4; i++) g->atv[i] = atv[i];
    g->cd_l = (int)(int16_t)PE_SpuRegister_LoadU16(SPU_CD_VOL_L);
    g->cd_r = (int)(int16_t)PE_SpuRegister_LoadU16(SPU_CD_VOL_R);
    g->mv_l = spu_fixed_volume(PE_SpuRegister_LoadU16(SPU_MAIN_VOL_L));
    g->mv_r = spu_fixed_volume(PE_SpuRegister_LoadU16(SPU_MAIN_VOL_R));
}

void PE_Xa_Mix(int16_t *stereo, unsigned frames)
{
    XaGain g;
    unsigned f;

    if (!stereo || frames == 0u)
        return;
    /* Need s_tail..s_tail+2 (newest tap) plus s_tail-1 history. */
    if (s_head <= s_tail + 2u)
        return;
    xa_resolve_gain(&g);
    if (!g.enabled)
        return;

    for (f = 0; f < frames; f++) {
        int16_t m1l, s0l, p1l, p2l;
        int16_t m1r, s0r, p1r, p2r;
        int xl, xr, ml, mr;
        unsigned i;

        while (s_phase >= PE_AUDIO_SAMPLE_RATE) {
            s_phase -= PE_AUDIO_SAMPLE_RATE;
            s_tail++;
            if (s_head <= s_tail + 2u)
                return;                      /* ring ran dry mid-call */
        }
        if (s_head <= s_tail + 2u)
            break;

        m1l = (s_tail > 0u) ? ring_at(s_tail - 1u, 0u) : (int16_t)0;
        s0l = ring_at(s_tail, 0u);
        p1l = ring_at(s_tail + 1u, 0u);
        p2l = ring_at(s_tail + 2u, 0u);
        m1r = (s_tail > 0u) ? ring_at(s_tail - 1u, 1u) : (int16_t)0;
        s0r = ring_at(s_tail, 1u);
        p1r = ring_at(s_tail + 1u, 1u);
        p2r = ring_at(s_tail + 2u, 1u);

        i = (unsigned)(((unsigned long long)s_phase * 256ull) / PE_AUDIO_SAMPLE_RATE);
        if (i > 255u) i = 255u;

        xl = PE_Xa_GaussInterp(m1l, s0l, p1l, p2l, i);
        xr = PE_Xa_GaussInterp(m1r, s0r, p1r, p2r, i);

        /* ATV matrix, unity = 80h (>>7). */
        ml = clamp16((xl * (int)g.atv[0] + xr * (int)g.atv[3]) >> 7);
        mr = clamp16((xl * (int)g.atv[1] + xr * (int)g.atv[2]) >> 7);
        ml = (ml * g.cd_l) >> 15;
        mr = (mr * g.cd_r) >> 15;
        ml = (ml * g.mv_l) >> 15;
        mr = (mr * g.mv_r) >> 15;

        stereo[f * 2u]     = clamp16(stereo[f * 2u] + ml);
        stereo[f * 2u + 1u] = clamp16(stereo[f * 2u + 1u] + mr);
        s_phase += s_src_rate;
    }
}

unsigned long long PE_Xa_SectorsDecoded(void) { return s_sectors; }
unsigned long long PE_Xa_FramesDecoded(void)  { return s_frames; }
unsigned PE_Xa_ActiveChannels(void)           { return s_channels; }
