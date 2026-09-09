/*
 * Phase AUD1-E0 — bounded host SPU synthesis from the native SPU-RAM image.
 *
 * Reads the same 512 KiB buffer and register window owned by pe_spu_dma.c.
 * This is not a cycle-accurate SPU emulator: ADPCM decode uses the standard
 * PSX filter table, linear interpolation (DEBT-SYS0-002), and ignores reverb
 * and Gaussian resampling. Key-on/off are driven by register writes at offsets
 * 0x188/0x18A relative to 0x1F801C00.
 */
#ifndef PE_SPU_SYNTH_H
#define PE_SPU_SYNTH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_SPU_SYNTH_SAMPLE_RATE 44100u
/* NTSC host frame quantum used by HostFB_VSync (44100 * 1001 / 60000). */
#define PE_SPU_SYNTH_FRAME_SAMPLES 736u

void PE_SpuSynth_Reset(void);
void PE_SpuSynth_OnRegisterWrite(uint32_t offset, uint16_t value);

/* Mix frame_samples interleaved stereo s16 frames into out_lr. */
void PE_SpuSynth_Render(int16_t *out_lr, unsigned frame_samples);

/* Diagnostics for native tests (not retail oracle material). */
uint64_t PE_SpuSynth_HashMix(unsigned frame_samples);
unsigned PE_SpuSynth_ActiveVoiceCount(void);
unsigned PE_SpuSynth_FramesRendered(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_SYNTH_H */
