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
 *     of 128 bytes per Form-2 sector), with the psx-spx filter tables.
 *   - Source rates 37800 Hz and 18900 Hz (the latter doubled), resampled to the
 *     host 44100 Hz sink with linear interpolation.
 *   - Sectors are classified by the Form-2 subheader submode (audio bit 2,
 *     0x04) and coded by the coding-info byte (stereo/rate/bits).
 *
 * There is no retail audio golden, so this is psx-spx + reference-implementation
 * semantics plus unit vectors, not claimed bit-exact retail parity.  The
 * 4-point gaussian interpolation the hardware uses is a documented gap; linear
 * interpolation is used instead.
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
 * resampling from the XA source rate and ADDING into the existing samples.
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

/* Decode one 128-byte 8-bit sound group for one channel.  Layout: four 28-byte
 * blocks; byte 0 of each block is the shift/filter parameter, bytes 1..27 are
 * signed 8-bit samples.  `blocks`/`prev`/`out` as above (up to 4*28 samples). */
unsigned PE_Xa_DecodeGroup8(const uint8_t group[128], const int *blocks,
                            unsigned block_count, int prev[2], int16_t *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_XA_H */
