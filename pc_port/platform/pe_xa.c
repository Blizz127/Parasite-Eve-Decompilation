/*
 * pe_xa.c — CD-XA ADPCM decoder + 44.1 kHz resampler for the native port.
 * See pe_xa.h for provenance and documented gaps.
 */
#include "pe_xa.h"
#include "pe_audio.h"

#include <string.h>

#define XA_PACKETS       18
#define XA_GROUP_BYTES   128
#define XA_BLOCK_SAMPLES 28
#define XA_RING_FRAMES   32768u

/* psx-spx / reference-implementation XA prediction filter tables. */
static const int kFiltPos[5] = { 0, 60, 115, 98, 122 };
static const int kFiltNeg[5] = { 0, 0, -52, -55, -60 };

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
static double   s_frac;
static unsigned long long s_sectors, s_frames;
static unsigned s_channels;

void PE_Xa_Reset(void)
{
    memset(s_prev_l, 0, sizeof s_prev_l);
    memset(s_prev_r, 0, sizeof s_prev_r);
    memset(s_ring, 0, sizeof s_ring);
    s_head = s_tail = 0u;
    s_src_rate = 37800u;
    s_frac = 0.0;
    s_sectors = s_frames = 0u;
    s_channels = 0u;
}

static void ring_push(int16_t l, int16_t r)
{
    if (s_head - s_tail >= XA_RING_FRAMES)
        s_tail++;                            /* drop oldest when saturated */
    unsigned i = (unsigned)(s_head % XA_RING_FRAMES);
    s_ring[i * 2u] = l;
    s_ring[i * 2u + 1u] = r;
    s_head++;
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
            int8_t raw = (int8_t)group[8 + blk * XA_BLOCK_SAMPLES + i];
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

void PE_Xa_Mix(int16_t *stereo, unsigned frames)
{
    double step;
    unsigned f;

    if (!stereo || frames == 0u)
        return;
    if (s_tail >= s_head)
        return;
    step = (double)s_src_rate / (double)PE_AUDIO_SAMPLE_RATE;

    for (f = 0; f < frames; f++) {
        int16_t l0, r0, l1, r1;
        int l, r;
        unsigned i0, i1;
        while (s_frac >= 1.0) {
            if (s_tail < s_head) s_tail++;
            s_frac -= 1.0;
        }
        if (s_tail >= s_head) {
            s_frac = 0.0;
            break;
        }
        i0 = (unsigned)(s_tail % XA_RING_FRAMES);
        i1 = (s_tail + 1u < s_head) ? (unsigned)((s_tail + 1u) % XA_RING_FRAMES) : i0;
        l0 = s_ring[i0 * 2u]; r0 = s_ring[i0 * 2u + 1u];
        l1 = s_ring[i1 * 2u]; r1 = s_ring[i1 * 2u + 1u];
        l = (int)l0 + (int)((double)((int)l1 - (int)l0) * s_frac);
        r = (int)r0 + (int)((double)((int)r1 - (int)r0) * s_frac);
        l += stereo[f * 2u];
        r += stereo[f * 2u + 1u];
        stereo[f * 2u]     = clamp16(l);
        stereo[f * 2u + 1u] = clamp16(r);
        s_frac += step;
    }
}

unsigned long long PE_Xa_SectorsDecoded(void) { return s_sectors; }
unsigned long long PE_Xa_FramesDecoded(void)  { return s_frames; }
unsigned PE_Xa_ActiveChannels(void)           { return s_channels; }
