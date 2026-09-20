/*
 * pe_spu.h — SPU voice playback model for the native port.
 *
 * The port previously stored SPU registers and copied DMA4 data into the SPU
 * RAM image, but never produced a sample.  This module consumes both: it
 * decodes SPU-ADPCM (psx-spx "Sample Data (SPU-ADPCM)"), advances each voice
 * with the pitch counter, applies the per-voice/main volumes and a shift/step
 * ADSR envelope, and mixes 24 voices to 16-bit stereo for pe_audio.
 *
 * Scope of this deliverable (documented gaps, not hidden):
 *   - 4-bit SPU-ADPCM decode with the psx-spx shift/filter prediction.
 *   - A 16-bit PCM block interpretation, used by capture/reverb-style buffers.
 *   - Pitch counter + LINEAR interpolation (the hardware's 4-point gaussian
 *     interpolation is a named gap; linear is exact at integer positions).
 *   - Fixed voice/main volumes.  Volume sweep mode is accepted but not swept.
 *   - Attack/Decay/Sustain/Release with the psx-spx shift/step algorithm.
 *   - Noise mode, pitch modulation and reverb are named gaps.
 *
 * No retail audio golden exists, so this is psx-spx semantics + unit vectors,
 * not claimed retail parity.
 */
#ifndef PE_SPU_H
#define PE_SPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_SPU_VOICE_COUNT  24
#define PE_SPU_BLOCK_BYTES  16
#define PE_SPU_BLOCK_SAMPLES 28  /* 4-bit blocks: 14 bytes x 2 nibbles */
#define PE_SPU_PCM16_SAMPLES  8  /* 16-bit block interpretation: 16 bytes / 2 */

/* Clear every voice (register file is owned by pe_spu_dma). */
void PE_Spu_Reset(void);

/* Register-write hook, called by PE_SpuRegister_StoreU16 after the value is
 * stored.  Interprets the key-on/key-off registers; all other writes only
 * become visible when the mixer reads the file. */
void PE_Spu_OnRegisterWrite(uint32_t offset, uint16_t value);

/* Mix `frames` interleaved stereo samples to the host sink (pe_audio). */
void PE_Spu_Render(int16_t *stereo, unsigned frames);

/* Render one vblank's worth (44100/60 samples) and push it to the open audio
 * sink.  Called once per presented frame by the host framebuffer. */
void PE_Spu_RenderVBlank(void);

/* Voices currently producing samples (diagnostics/tests). */
int PE_Spu_ActiveVoiceCount(void);

/* Guest-drive diagnostics: register writes seen and voices keyed on/off. */
void PE_Spu_GetStats(unsigned *reg_writes, unsigned *key_ons, unsigned *key_offs);

/* Synthetic pipeline self-test: mix `frames` samples of a looped ADPCM tone
 * through the real mixer/sink.  NOT retail audio; for audible path checks. */
void PE_Spu_SelfTest(unsigned frames);

/* Decode one 16-byte SPU-ADPCM block.  `prev1`/`prev2` are the running
 * prediction history and are updated in place.  `out` receives 28 samples.
 * Returns the block's flag byte bits 0..2 packed in *flags_out (bit0=end,
 * bit1=repeat, bit2=loop-start).  Exposed for unit vectors. */
int PE_Spu_AdpcmDecode4(const uint8_t block[PE_SPU_BLOCK_BYTES],
                        int16_t out[PE_SPU_BLOCK_SAMPLES],
                        int *prev1, int *prev2, unsigned *flags_out);

/* Interpret a 16-byte block as 8 little-endian signed 16-bit PCM samples.
 * This is the "16-bit sample" path (capture/reverb buffers), not ADPCM. */
void PE_Spu_AdpcmDecode16(const uint8_t block[PE_SPU_BLOCK_BYTES],
                          int16_t out[PE_SPU_PCM16_SAMPLES]);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_H */
