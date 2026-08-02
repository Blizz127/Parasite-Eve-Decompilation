/*
 * Minimal X11 window — no headers required, links against -lX11.
 * Shows the framebuffer on the desktop so you can SEE the black frame.
 */
#ifndef HOST_WINDOW_H
#define HOST_WINDOW_H

#include <stdint.h>

/* Open a 320×240 window on the given display, return 0 on success */
int  HostWindow_Open(const char *display, int width, int height);
/* Blit RGB pixels to the window */
void HostWindow_Blit(const uint8_t *rgb, int width, int height);
/* Flush and keep the window alive for N milliseconds */
void HostWindow_Show(int milliseconds);
/* Close the window */
void HostWindow_Close(void);

#endif
