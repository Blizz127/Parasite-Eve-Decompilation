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
void HostFB_DrawSync(int mode);
int  HostFB_WritePPM(const char *path);
const uint8_t *HostFB_GetPixels(void);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask);

#endif
