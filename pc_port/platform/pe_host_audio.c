/*
 * PulseAudio simple playback via dlopen.  Fails closed to a no-op when the
 * library or a default sink is unavailable (headless CI, Windows builds).
 */
#include "pe_host_audio.h"
#include "pe_spu_synth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
/* Win32 port uses a separate window backend; host PCM is Linux-first here. */
#else
#include <dlfcn.h>
#endif

typedef struct pa_sample_spec {
    int format;
    uint32_t rate;
    uint8_t channels;
} pa_sample_spec;

typedef struct pa_buffer_attr {
    uint32_t maxlength;
    uint32_t tlength;
    uint32_t prebuf;
    uint32_t minreq;
    uint32_t fragsize;
} pa_buffer_attr;

typedef void *pa_simple;

enum {
    PA_SAMPLE_S16LE = 3
};

#if !defined(_WIN32)
typedef pa_simple *(*pa_simple_new_t)(const char *, const char *, int,
                                       const char *, const char *,
                                       const pa_sample_spec *,
                                       const pa_buffer_attr *, int *, char **);
typedef int (*pa_simple_write_t)(pa_simple *, const void *, size_t, int *);
typedef void (*pa_simple_free_t)(pa_simple *);
#endif

static int g_loaded;
static int g_active;
#if !defined(_WIN32)
static void *g_pulse;
static pa_simple_new_t g_pa_simple_new;
static pa_simple_write_t g_pa_simple_write;
static pa_simple_free_t g_pa_simple_free;
static pa_simple *g_stream;
#endif

static int env_disabled(void)
{
    const char *v = getenv("PE_AUDIO_DISABLE");
    return v && v[0] && v[0] != '0';
}

void PE_HostAudio_Init(void)
{
#if defined(_WIN32)
    g_loaded = 0;
#else
    if (env_disabled()) return;
    g_pulse = dlopen("libpulse-simple.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_pulse)
        g_pulse = dlopen("libpulse.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_pulse) return;
    g_pa_simple_new = (pa_simple_new_t)dlsym(g_pulse, "pa_simple_new");
    g_pa_simple_write = (pa_simple_write_t)dlsym(g_pulse, "pa_simple_write");
    g_pa_simple_free = (pa_simple_free_t)dlsym(g_pulse, "pa_simple_free");
    if (!g_pa_simple_new || !g_pa_simple_write || !g_pa_simple_free) {
        dlclose(g_pulse);
        g_pulse = NULL;
        return;
    }
    g_loaded = 1;
#endif
}

void PE_HostAudio_Shutdown(void)
{
    PE_HostAudio_Stop();
#if !defined(_WIN32)
    if (g_pulse) {
        dlclose(g_pulse);
        g_pulse = NULL;
    }
#endif
    g_loaded = 0;
}

int PE_HostAudio_Start(void)
{
    if (g_active || !g_loaded || env_disabled()) return -1;
#if defined(_WIN32)
    return -1;
#else
    pa_sample_spec spec;
    int err = 0;
    spec.format = PA_SAMPLE_S16LE;
    spec.rate = PE_SPU_SYNTH_SAMPLE_RATE;
    spec.channels = 2;
    g_stream = g_pa_simple_new(NULL, "Parasite Eve Port", 1, NULL,
                               "game", &spec, NULL, &err, NULL);
    if (!g_stream) return -1;
    g_active = 1;
    return 0;
#endif
}

void PE_HostAudio_Stop(void)
{
#if !defined(_WIN32)
    if (g_stream && g_pa_simple_free)
        g_pa_simple_free(g_stream);
    g_stream = NULL;
#endif
    g_active = 0;
}

void PE_HostAudio_Submit(const int16_t *interleaved_lr, unsigned frame_count)
{
    if (!g_active || !interleaved_lr || frame_count == 0u) return;
#if !defined(_WIN32)
    if (!g_stream || !g_pa_simple_write) return;
    size_t bytes = (size_t)frame_count * 2u * sizeof(int16_t);
    int err = 0;
    if (g_pa_simple_write(g_stream, interleaved_lr, bytes, &err) < 0) {
        fprintf(stderr, "PE_HostAudio: pa_simple_write failed (%d)\n", err);
        PE_HostAudio_Stop();
    }
#endif
}

int PE_HostAudio_Active(void)
{
    return g_active;
}
