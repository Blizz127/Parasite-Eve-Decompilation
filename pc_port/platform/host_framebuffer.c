/*
 * Phase 6A — Host framebuffer (headless, no SDL2/OpenGL).
 *
 * 320×240 logical pixels, RGB 8:8:8 per channel.
 * Deterministic: no GPU, no vsync, no timing jitter.
 * Output: PPM (Portable Pixmap) file on request.
 */

#include "host_framebuffer.h"
#include "game_port.h"
#include "host_vram.h"
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "pe_spu_dma.h"
#include "pe_sdk.h"
#include "pe_cdreg.h"
#include "pe_mdec.h"
#include "pe_irq_delivery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── framebuffer state ───────────────────────────────────────────────── */

static uint8_t fb[PE_PORT_FB_WIDTH * PE_PORT_FB_HEIGHT * 3];  /* RGB */
static int     fb_presented = 0;
static int     fb_mask      = 0;   /* 0 = blanked, 1 = visible (SetDispMask) */
static uint32_t fb_device_cycles = 0;
static int     fb_vsync_count = 0;
static int     fb_drawsync_count = 0;

/* ── public API ───────────────────────────────────────────────────────── */

void HostFB_Init(void)
{
    memset(fb, 0, sizeof(fb));
    fb_presented = 0;
    fb_mask      = 0;
    fb_vsync_count = 0;
    fb_device_cycles = 0;
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
    PE_Port_InvokePresentHook();
}

void HostFB_PresentDispEnv(pe_addr_t env)
{
    uint32_t dx = (uint32_t)PE_LoadU16(env);
    uint32_t dy = (uint32_t)PE_LoadU16(env + 2u);
    uint32_t dw = (uint32_t)PE_LoadU16(env + 4u);
    /* Original PutDispEnv 75858..75880 and 75A14..75A60: NTSC
     * screen.y is relative to scanline16; screen.h==0 means240 lines.
     * The source rectangle height selects the GPU mode, not this range. */
    int start = (int16_t)PE_LoadU16(env + 10u) + 16;
    int lines = (int16_t)PE_LoadU16(env + 14u);
    int end = start + (lines ? lines : 240);
    if (start < 16) start = 16;
    if (start > 257) start = 257;
    if (end < start + 2) end = start + 2;
    else if (end > 258) end = 258;
    uint32_t top = (uint32_t)(start - 16);
    uint32_t dh = (uint32_t)(end - start);
    uint32_t row, col;

    if (!PE_Port_FramePresentationAllowed()) return;
    /* Blank scanlines outside this frame's display range. Keeping the
     * old framebuffer there leaks pixels when a scene narrows the range. */
    memset(fb, 0, sizeof(fb));
    if (fb_mask) {
        if (dw > (uint32_t)PE_PORT_FB_WIDTH)  dw = (uint32_t)PE_PORT_FB_WIDTH;
        for (row = 0u; row < dh && top + row < (uint32_t)PE_PORT_FB_HEIGHT; row++) {
            for (col = 0u; col < dw; col++) {
                uint16_t pixel = 0u;
                uint8_t *dst = fb + ((size_t)(top + row) * PE_PORT_FB_WIDTH + col) * 3u;
                if (PE_GPU_ReadVRAM(dx + col, dy + row, &pixel)) {
                    HostVRAM_DecodePixel(pixel, dst);
                } else {
                    dst[0] = 0u;
                    dst[1] = 0u;
                    dst[2] = 0u;
                }
            }
        }
    }
    fb_presented++;
    PE_Port_FramePresented(fb_presented);
    PE_Port_InvokePresentHook();
}

void HostFB_SetDispMask(int mask)
{
    fb_mask = mask;
}

static void HostFB_ServiceDeviceIrq(void)
{
    if(!PE_Port_ShouldStop() && !PE_LoadU16(0x800945E6u) && (PE_IRQ_ReadStatus()&PE_IRQ_GetMask()))
        (void)PE_IRQ_ServicePendingForGeneration(PE_IRQ_Generation());
}
static void HostFB_DeviceTime(uint32_t cycles)
{
    /* NTSC host approximation:33868800 CPU cycles/60 frames. Device
     * responses are serviced before the next generated VBlank edge. */
    PE_CdReg_ServiceDevice(cycles);HostFB_ServiceDeviceIrq();
    if(PE_Port_ShouldStop()) return;
    fb_device_cycles+=cycles;
    while(fb_device_cycles>=564480u) {
        fb_device_cycles-=564480u;
        PeIrqGeneration generation=PE_IRQ_Generation();
        PE_GPU_VBlankStep();
        (void)PE_GPU_SetVBlank(0,generation);
        (void)PE_GPU_SetVBlank(1,generation);
        HostFB_ServiceDeviceIrq();
        (void)PE_GPU_SetVBlank(0,generation);
        if(PE_Port_ShouldStop()) return;
    }
}

void HostFB_VSync(int mode)
{
    /* A nonblocking audio upload must complete even when the game only
     * polls its completion flag. Service DMA at the normal host tick. */
    (void)PE_SpuDma_Service();
    /* While a movie decode is in flight the GPU DMA2 upload of the
     * previous slice completes in parallel with the MDEC on hardware.
     * The host tick is the asynchronous-progress stand-in, so complete
     * the pending GPU transfer (and drain its queued ring through the
     * DMA IRQ checkpoint) before the MDEC decodes into the rotating
     * buffer again. Without this, queued LoadImages execute late and
     * read overwritten slice buffers. */
    if(PE_MDEC_HasDecode() && PE_GPU_DMA2Pending())
        (void)PE_Port_ServiceDmaIrqCheckpoint();
    (void)PE_MDEC_Service();
    if(PE_MDEC_HasDecode()) HostFB_ServiceDeviceIrq();
    /* Deterministic host CPU-work quantum, including busy counter queries.
     * This is an approximate device clock, not a cycle-accurate CPU model. */
    int device=PE_CdReg_DeviceEnabled();
    if(device && (mode<0 || mode==1)) HostFB_DeviceTime(1024u);
    if (mode >= 0) PE_Event_ServiceAudioCommands();
    /* Negative VSync is a counter query and mode 1 queries elapsed scan
     * lines. Waiting modes advance the shared 60 Hz clock used by menus. */
    if (mode>=0 && mode!=1) {
        int ticks=mode>0?mode:1;
        while (ticks-->0) {
            if(device) HostFB_DeviceTime(564480u);
            else PE_GPU_VBlankStep();
            if(PE_Port_ShouldStop()) break;
        }
    }
    fb_vsync_count++;
    (void)mode;
}

void HostFB_PumpCdProgress(void)
{
    /* Same Spu/MDEC preamble as VSync, then one non-XA sector period so a
     * single E0 poll can retire a pending CD sector (pe_cdreg
     * CdSectorCycles: 451584 non-XA / 225792 XA). */
    (void)PE_SpuDma_Service();
    if(PE_MDEC_HasDecode() && PE_GPU_DMA2Pending())
        (void)PE_Port_ServiceDmaIrqCheckpoint();
    (void)PE_MDEC_Service();
    if(PE_MDEC_HasDecode()) HostFB_ServiceDeviceIrq();
    if(PE_CdReg_DeviceEnabled())
        HostFB_DeviceTime(451584u);
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
