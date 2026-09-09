/* Host framebuffer — headless, PPM output */
#ifndef HOST_FRAMEBUFFER_H
#define HOST_FRAMEBUFFER_H

#include <stdint.h>

#include "pe_guest_ram.h"

#define PE_PORT_FB_WIDTH  320
#define PE_PORT_FB_HEIGHT 240

void HostFB_Init(void);
void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void HostFB_Present(void);
/* Phase 6E-PRS1 — display presentation.  Reads the DISPENV disp RECT
 * (x/y/w/h halfwords at env+0/2/4/6) from guest RAM and copies that VRAM
 * window into the host framebuffer, decoded through the GPU VRAM
 * authority.  Mask off presents black (retail blanks).  Read-only on
 * guest state; keeps the present counter and frame-limit budget. */
void HostFB_PresentDispEnv(pe_addr_t env);
void HostFB_SetDispMask(int mask);
void HostFB_VSync(int mode);
/* Sector-scale CD/DMA host tick for movie E0 waits. VSync(-1) only advances
 * 1024 device cycles; one CD sector costs 225792 (XA) or 451584 (non-XA).
 * Use this when spinning until D_800B89F4 marks a last video chunk so the
 * E0 2000-try window can finish a multi-sector STR frame. Does not bump the
 * VSync counter (retail's VSync(-1) is a query).
 * DAY2-158x: stall while B0CD0+pending & DMA1 busy; idle-DMA1 catch-up calls
 * 7C564 (91DC8 miss / B0DBB gate); DMA1-busy timeout → CD_B0CD0_dma1_starved. */
void HostFB_PumpCdProgress(void);
void HostFB_DrawSync(int mode);
int  HostFB_WritePPM(const char *path);
const uint8_t *HostFB_GetPixels(void);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask);

#endif
