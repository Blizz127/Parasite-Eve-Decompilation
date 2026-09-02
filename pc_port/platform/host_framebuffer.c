/*
 * Phase 6A — Host framebuffer (headless, no SDL2/OpenGL).
 *
 * 320×240 logical pixels, RGB 8:8:8 per channel.
 * Deterministic: no GPU, no vsync, no timing jitter.
 * Output: PPM (Portable Pixmap) file on request.
 */

#include "host_framebuffer.h"
#include "pe_pad.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── framebuffer state ───────────────────────────────────────────────── */

static uint8_t fb[PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3];  /* RGB */
static int     fb_presented = 0;
static int     fb_mask      = 0;   /* 0 = blanked, 1 = visible (SetDispMask) */
static int     fb_vsync_count = 0;
static int     fb_drawsync_count = 0;

/* ── public API ───────────────────────────────────────────────────────── */

void HostFB_Init(void)
{
    memset(fb, 0, sizeof(fb));
    fb_presented = 0;
    fb_mask      = 0;
    fb_vsync_count = 0;
    fb_drawsync_count = 0;
}

void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
{
    /* Clamp to framebuffer bounds */
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > PE_PORT_FB_WIDTH)  w = PE_PORT_FB_WIDTH  - x;
    if (y + h > PE_PORT_FB_HEIGHT) h = PE_PORT_FB_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    for (int row = y; row < y + h; row++) {
        uint8_t *dst = fb + (row * PE_PORT_FB_WIDTH + x) * 3;
        for (int col = 0; col < w; col++) {
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;
            dst += 3;
        }
    }
}

void HostFB_Present(void)
{
    if (!PE_Port_FramePresentationAllowed()) return;
    fb_presented++;
    PE_Port_FramePresented(fb_presented);
}

void HostFB_SetDispMask(int mask)
{
    fb_mask = mask;
}

void HostFB_VSync(int mode)
{
    fb_vsync_count++;
    /* B54K-AT: the retail pad driver refreshes its buffers per VSync. */
    PE_Pad_Deliver((uint32_t)fb_vsync_count);
    (void)mode;
}

void HostFB_DrawSync(int mode)
{
    fb_drawsync_count++;
    (void)mode;
}

int HostFB_WritePPM(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    fprintf(f, "P6\n%d %d\n255\n", PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
    fwrite(fb, 1, sizeof(fb), f);
    fclose(f);
    return 0;
}

const uint8_t *HostFB_GetPixels(void)
{
    return fb;
}

void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask)
{
    if (vsync)     *vsync     = fb_vsync_count;
    if (drawsync)  *drawsync  = fb_drawsync_count;
    if (presented) *presented = fb_presented;
    if (mask)      *mask      = fb_mask;
}
