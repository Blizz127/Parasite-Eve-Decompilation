/*
 * pe_xa.h — CD-XA ADPCM audio for the native port.
 *
 * The port previously treated CD-XA audio as out of scope: the drive model
 * delivered raw sectors and the SPU mixer produced silence because the retail
 * main EXE keys no voices in gameplay.  FMV and music audio on the disc is
 * CD-XA ADPCM interleaved into the stream, decoded by the drive's CXD1199
 * decoder — this module is that decoder plus the resampler into the host sink.
 *
 * Scope / provenance:
 *   - 4-bit and 8-bit XA-ADPCM per the standard sound-group layout (18 groups
 *     of 128 bytes per Form-2 sector), with the psx-spx filter tables.  The
 *     8-bit layout is 28 little-endian 32-bit words at +16 (byte N of each
 *     word is sub-block N); the parameter bytes are at +4.
 *   - Source rates 37800 Hz and 18900 Hz, resampled to the host 44100 Hz sink
 *     with the PS1 4-tap gaussian interpolation filter (the psx-spx SPU
 *     512-entry `gauss` table and its oldest/older/old/newest tap ordering,
 *     cross-checked against DuckStation's identical table).
 *   - Sector classification and decode from the Form-2 subheader submode
 *     (audio bit 2, 0x04) and the coding-info byte (stereo/rate/bits).
 *   - Output volume chain: the CD controller ATV0..3 matrix (psx-spx
 *     "ATV0 L->L / ATV1 L->R / ATV2 R->R / ATV3 R->L", 80h = unity), the SPU
 *     CD audio input volume (1F801DB0h/DB2h) and the SPU main volume
 *     (1F801D80h/82h, already applied to the voice mix by pe_spu), gated by
 *     SPUCNT CD-audio-enable (bit0) and mute (bit14).
 *
 * There is no retail audio golden, so this is psx-spx + reference-implementation
 * semantics plus unit vectors, not claimed bit-exact retail parity.  Known
 * remaining gaps: emphasis filter, and no noise/pitch-mod/reverb modelling.
 *
 * Host-only state.  Consuming a sector never touches guest RAM.
 */
#ifndef PE_XA_H
#define PE_XA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reset the decoder state (per-channel prediction history and the PCM ring). */
void PE_Xa_Reset(void);

/* Consume one raw 2352-byte sector.  If the sector carries XA audio (Form-2
 * submode bit 2), the 18 sound groups are decoded and appended to the PCM
 * ring.  Returns the number of source frames appended (0 if not audio). */
unsigned PE_Xa_ConsumeSector(const uint8_t raw[2352]);

/* Mix up to `frames` interleaved stereo samples into `stereo` at 44100 Hz,
 * resampling from the XA source rate with the PS1 gaussian interpolator and
 * applying the CD-XA volume chain, and ADDING into the existing samples.
 * Safe with a NULL/closed sink; does nothing when the ring is empty. */
void PE_Xa_Mix(int16_t *stereo, unsigned frames);

/* Diagnostics. */
unsigned long long PE_Xa_SectorsDecoded(void);
unsigned long long PE_Xa_FramesDecoded(void);
unsigned PE_Xa_ActiveChannels(void);

/* Decode one 128-byte 4-bit sound group for one channel.  `blocks` lists the
 * sub-block indices (0..7); mono uses all eight, stereo the even/odd four.
 * `prev[0]`/`prev[1]` are the running prediction history (updated in place).
 * Writes up to 8*28 samples and returns the count.  Exposed for unit vectors. */
unsigned PE_Xa_DecodeGroup4(const uint8_t group[128], const int *blocks,
                            unsigned block_count, int prev[2], int16_t *out);

/* Decode one 128-byte 8-bit sound group for one channel.  Layout: parameter
 * bytes at +4, then 28 little-endian 32-bit words at +16; byte `blk` of each
 * word is sub-block `blk`'s signed sample.  `blocks`/`prev`/`out` as above
 * (up to 4*28 samples). */
unsigned PE_Xa_DecodeGroup8(const uint8_t group[128], const int *blocks,
                            unsigned block_count, int prev[2], int16_t *out);

/* PS1 4-point gaussian interpolation (psx-spx SPU chapter).  `oldest`/`older`/
 * `old`/`newest` are four consecutive input samples, `index` is the 00h..FFh
 * phase (pitch-counter bits 4..11).  Exposed for unit vectors. */
int16_t PE_Xa_GaussInterp(int oldest, int older, int old, int newest,
                          unsigned index);

#ifdef __cplusplus
}
#endif

#endif /* PE_XA_H */
