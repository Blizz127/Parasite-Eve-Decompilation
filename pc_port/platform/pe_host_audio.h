/*
 * Phase AUD1-E0 — optional host PCM output (PulseAudio via dlopen on Linux).
 *
 * When the simple output device cannot be opened, submission is a silent no-op
 * and synthesis tests still run headlessly.
 */
#ifndef PE_HOST_AUDIO_H
#define PE_HOST_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void PE_HostAudio_Init(void);
void PE_HostAudio_Shutdown(void);

/* Open the default playback stream at PE_SPU_SYNTH_SAMPLE_RATE. */
int  PE_HostAudio_Start(void);
void PE_HostAudio_Stop(void);

/* interleaved stereo s16; frame_count stereo pairs. */
void PE_HostAudio_Submit(const int16_t *interleaved_lr, unsigned frame_count);

int  PE_HostAudio_Active(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_HOST_AUDIO_H */
