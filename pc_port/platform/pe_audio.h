/*
 * pe_audio.h — host audio sink for the native port.
 *
 * The port had no audio output path at all: SPU DMA writes landed in the SPU
 * RAM image and SPU register writes landed in a register file, but nothing
 * consumed either.  This module is the output side: a tiny, testable sink
 * abstraction that the SPU mixer (pe_spu.c) pushes interleaved stereo
 * 16-bit PCM into.
 *
 * Sinks:
 *   PE_AUDIO_SINK_NULL — drop samples (default; keeps headless runs side-effect
 *                        free and every test deterministic).
 *   PE_AUDIO_SINK_WAV  — write a RIFF/WAVE file (16-bit stereo 44100 Hz).
 *                        The size fields are patched after every block, so a
 *                        file stays playable even if the run is killed.
 *   PE_AUDIO_SINK_LIVE — a real-time host device.  No audio dev library is
 *                        guaranteed on the build machine, so this is compiled
 *                        behind PE_AUDIO_HAVE_LIVE and otherwise reports
 *                        unavailable; it never breaks the build.
 *
 * The sink is host-only state.  Nothing here touches guest RAM.
 */
#ifndef PE_AUDIO_H
#define PE_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PE_AUDIO_SAMPLE_RATE 44100u
#define PE_AUDIO_CHANNELS    2u

typedef enum PeAudioSinkKind {
    PE_AUDIO_SINK_NULL = 0,
    PE_AUDIO_SINK_WAV  = 1,
    PE_AUDIO_SINK_LIVE = 2
} PeAudioSinkKind;

/* Open a sink.  `path` is required for WAV (NULL means build/pe_audio.wav).
 * Returns 1 on success, 0 on failure (a failed WAV open leaves the sink shut,
 * never half-open).  Re-opening replaces the current sink. */
int  PE_Audio_Init(PeAudioSinkKind kind, const char *path);

/* Flush and close the current sink.  Safe when already closed. */
void PE_Audio_Shutdown(void);

int             PE_Audio_IsOpen(void);
PeAudioSinkKind PE_Audio_Kind(void);
const char     *PE_Audio_Path(void);

/* Push `frames` interleaved stereo samples (L,R,L,R,...). */
void PE_Audio_Write(const int16_t *samples, unsigned frames);

/* Total frames written since the sink was opened (diagnostics/tests). */
unsigned long long PE_Audio_FramesWritten(void);

/* 1 when a real-time backend is compiled in and usable. */
int PE_Audio_LiveBackendAvailable(void);

/* Select the sink from the environment: PE_AUDIO_WAV=<path> opens a WAV sink;
 * otherwise the sink stays NULL.  Called once at port startup. */
void PE_Audio_InitFromEnv(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_AUDIO_H */
