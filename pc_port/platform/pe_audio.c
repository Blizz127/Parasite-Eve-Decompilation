/*
 * pe_audio.c — see pe_audio.h.
 */
#include "pe_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A real-time backend needs an audio dev library that may not exist here, so it
 * is opt-in at compile time.  The stub keeps the build green and honest. */
#if defined(PE_AUDIO_HAVE_LIVE)
#error "PE_AUDIO_HAVE_LIVE backend not implemented in this deliverable"
#endif

typedef struct {
    PeAudioSinkKind kind;
    FILE *fp;
    char  path[512];
    unsigned long long frames;
    int     open;
} PeAudioSink;

static PeAudioSink g_sink;

static void PutU32LE(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void PutU16LE(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static int WriteWavHeader(FILE *fp)
{
    uint8_t hdr[44];
    memcpy(hdr, "RIFF", 4);
    PutU32LE(hdr + 4, 36u);                 /* patched as data grows */
    memcpy(hdr + 8, "WAVE", 4);
    memcpy(hdr + 12, "fmt ", 4);
    PutU32LE(hdr + 16, 16u);                /* PCM fmt chunk size */
    PutU16LE(hdr + 20, 1u);                 /* PCM */
    PutU16LE(hdr + 22, (uint16_t)PE_AUDIO_CHANNELS);
    PutU32LE(hdr + 24, PE_AUDIO_SAMPLE_RATE);
    PutU32LE(hdr + 28, PE_AUDIO_SAMPLE_RATE * PE_AUDIO_CHANNELS * 2u);
    PutU16LE(hdr + 32, (uint16_t)(PE_AUDIO_CHANNELS * 2u)); /* block align */
    PutU16LE(hdr + 34, 16u);                /* bits per sample */
    memcpy(hdr + 36, "data", 4);
    PutU32LE(hdr + 40, 0u);                 /* patched as data grows */
    return fwrite(hdr, 1, sizeof(hdr), fp) == sizeof(hdr);
}

/* Patch the two size fields so the file is valid after every block. */
static void PatchWavSizes(FILE *fp, unsigned long long frames)
{
    uint8_t buf[4];
    uint32_t data_bytes = (uint32_t)(frames * PE_AUDIO_CHANNELS * 2ull);
    if (fseek(fp, 4, SEEK_SET) != 0) return;
    PutU32LE(buf, 36u + data_bytes);
    (void)fwrite(buf, 1, 4, fp);
    if (fseek(fp, 40, SEEK_SET) != 0) return;
    PutU32LE(buf, data_bytes);
    (void)fwrite(buf, 1, 4, fp);
    (void)fseek(fp, 0, SEEK_END);
}

int PE_Audio_Init(PeAudioSinkKind kind, const char *path)
{
    PE_Audio_Shutdown();

    if (kind == PE_AUDIO_SINK_NULL) {
        g_sink.kind = kind;
        g_sink.open = 1;
        g_sink.frames = 0;
        g_sink.path[0] = '\0';
        return 1;
    }

    if (kind == PE_AUDIO_SINK_WAV) {
        const char *p = (path && path[0]) ? path : "build/pe_audio.wav";
        FILE *fp = fopen(p, "wb");
        if (!fp) return 0;
        if (!WriteWavHeader(fp)) {
            fclose(fp);
            return 0;
        }
        g_sink.kind = kind;
        g_sink.fp = fp;
        g_sink.open = 1;
        g_sink.frames = 0;
        snprintf(g_sink.path, sizeof(g_sink.path), "%s", p);
        return 1;
    }

    /* LIVE: no backend compiled in. */
    return 0;
}

void PE_Audio_Shutdown(void)
{
    if (g_sink.fp) {
        PatchWavSizes(g_sink.fp, g_sink.frames);
        fclose(g_sink.fp);
    }
    memset(&g_sink, 0, sizeof(g_sink));
}

int PE_Audio_IsOpen(void) { return g_sink.open; }
PeAudioSinkKind PE_Audio_Kind(void) { return g_sink.kind; }
const char *PE_Audio_Path(void) { return g_sink.path; }

void PE_Audio_Write(const int16_t *samples, unsigned count)
{
    if (!g_sink.open || count == 0u || !samples) return;
    g_sink.frames += count;
    if (g_sink.kind == PE_AUDIO_SINK_WAV && g_sink.fp) {
        (void)fwrite(samples, sizeof(int16_t) * PE_AUDIO_CHANNELS, count, g_sink.fp);
        PatchWavSizes(g_sink.fp, g_sink.frames);
    }
    /* NULL (and a future live sink that dropped the block) writes nothing. */
}

unsigned long long PE_Audio_FramesWritten(void) { return g_sink.frames; }

int PE_Audio_LiveBackendAvailable(void)
{
#ifdef PE_AUDIO_HAVE_LIVE
    return 1;
#else
    return 0;
#endif
}

void PE_Audio_InitFromEnv(void)
{
    const char *wav = getenv("PE_AUDIO_WAV");
    if (wav && wav[0]) {
        if (!PE_Audio_Init(PE_AUDIO_SINK_WAV, wav)) {
            fprintf(stderr, "[AUDIO] could not open WAV sink '%s'\n", wav);
        } else {
            fprintf(stderr, "[AUDIO] WAV sink '%s' (%u Hz stereo s16)\n",
                    PE_Audio_Path(), PE_AUDIO_SAMPLE_RATE);
        }
    }
}
