/*
 * Phase 6B — Native X11 window backend (dlopen, no dev headers).
 *
 * Loads libX11.so.6 at runtime.  No manual struct layouts — we use
 * only function pointers and primitive types (Window = unsigned long).
 * The XImage buffer is caller-allocated.
 *
 * Backend: libX11 via dlopen (SDL2 preferred but unavailable on this VPS).
 */

#ifndef HOST_WINDOW_H
#define HOST_WINDOW_H

#include <stdint.h>

/* Open a window.  width/height are the output (client) size.
 * title sets the window title bar.  scale is the integer zoom factor.
 * Returns 0 on success, -1 on failure. */
int  HostWindow_Open(const char *display, int width, int height,
                     const char *title, int scale);

/* Upload RGB 8:8:8 framebuffer pixels to the window.
 * fb_w/fb_h are the SOURCE framebuffer dimensions (always 320×240).
 * Nearest-neighbor scaling to the window size. */
void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h);

/* Process pending X11 events.  Returns:
 *  0 = normal
 *  1 = close requested (Escape or window-close) */
int  HostWindow_Poll(void);

/* Update the window title (used for debug overlay). */
void HostWindow_SetTitle(const char *title);

/* Run the event loop for a fixed number of milliseconds, or until
 * close is requested.  Returns 0 on timeout, 1 on close. */
int  HostWindow_Run(int milliseconds);

/* Close the window and release resources. */
void HostWindow_Close(void);

/* Nonzero when a window was successfully opened. */
extern int g_host_window_open;

#endif
