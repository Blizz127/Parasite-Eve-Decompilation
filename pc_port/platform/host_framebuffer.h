/* Host framebuffer — headless, PPM output */
#ifndef HOST_FRAMEBUFFER_H
#define HOST_FRAMEBUFFER_H

#include <stdint.h>

#define PE_PORT_FB_WIDTH  320
#define PE_PORT_FB_HEIGHT 240

void HostFB_Init(void);
void HostFB_ClearImage(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void HostFB_Present(void);
void HostFB_SetDispMask(int mask);
void HostFB_VSync(int mode);
void HostFB_DrawSync(int mode);
int  HostFB_WritePPM(const char *path);
const uint8_t *HostFB_GetPixels(void);
void HostFB_GetState(int *vsync, int *drawsync, int *presented, int *mask);

#endif
