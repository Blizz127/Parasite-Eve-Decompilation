/*
 * Minimal X11 window — links against -lX11 directly, no dev headers.
 * Declares only the symbols we need (XOpenDisplay, XCreateSimpleWindow, etc.)
 *
 * This is the simplest possible X11 window: open display, create window,
 * show it, blit framebuffer with XPutImage, sleep, close.
 */
#include "host_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

/* ── Xlib types (minimal) ───────────────────────────────────────────── */
typedef unsigned long XID;
typedef XID Window;
typedef XID Drawable;
typedef XID Pixmap;
typedef XID Colormap;
typedef XID GC;
typedef XID VisualID;

typedef struct {
    int x, y, width, height;
    int border_width, depth;
    /* ... truncated; we only need sizeof */
} XWindowAttributes;

typedef struct {
    int width, height, depth;
    /* ... */
} XPixmapFormatValues;

typedef struct {
    void *ext_data;
    int depth, bits_per_pixel, scanline_pad;
    /* ... */
} XImage;

/* ── Function pointers we load from libX11 ──────────────────────────── */
static void *g_xlib = NULL;
static void *(*XOpenDisplay)(const char *) = NULL;
static int   (*XCloseDisplay)(void *) = NULL;
static int   (*XDefaultScreen)(void *) = NULL;
static Window (*XDefaultRootWindow)(void *) = NULL;
static Window (*XCreateSimpleWindow)(void *, Window, int, int, unsigned, unsigned,
                                      unsigned, unsigned, unsigned) = NULL;
static int   (*XMapWindow)(void *, Window) = NULL;
static int   (*XStoreName)(void *, Window, const char *) = NULL;
static GC     (*XCreateGC)(void *, Drawable, unsigned long, void *) = NULL;
static int   (*XFreeGC)(void *, GC) = NULL;
static XImage *(*XCreateImage)(void *, void *, unsigned, int, int, char *, unsigned, unsigned, int, int) = NULL;
static int   (*XPutImage)(void *, Drawable, GC, XImage *, int, int, int, int, unsigned, unsigned) = NULL;
static int   (*XFlush)(void *) = NULL;
static int   (*XDestroyWindow)(void *, Window) = NULL;
static int   (*XDestroyImage)(XImage *) = NULL;

static void *g_dpy = NULL;
static Window g_win = 0;
static GC g_gc = 0;
static XImage *g_ximg = NULL;
static uint8_t *g_imgbuf = NULL;

/* runtime linking */
static void *xlib_sym(const char *name) {
    void *p = dlsym(g_xlib, name);
    if (!p) { fprintf(stderr, "[WINDOW] missing symbol: %s\n", name); exit(1); }
    return p;
}

int HostWindow_Open(const char *display, int width, int height)
{
    g_xlib = dlopen("libX11.so.6", RTLD_LAZY);
    if (!g_xlib) { fprintf(stderr, "[WINDOW] libX11.so.6 not found\n"); return -1; }

    XOpenDisplay        = xlib_sym("XOpenDisplay");
    XCloseDisplay       = xlib_sym("XCloseDisplay");
    XDefaultScreen      = xlib_sym("XDefaultScreen");
    XDefaultRootWindow  = xlib_sym("XDefaultRootWindow");
    XCreateSimpleWindow = xlib_sym("XCreateSimpleWindow");
    XMapWindow          = xlib_sym("XMapWindow");
    XStoreName          = xlib_sym("XStoreName");
    XCreateGC           = xlib_sym("XCreateGC");
    XFreeGC             = xlib_sym("XFreeGC");
    XCreateImage        = xlib_sym("XCreateImage");
    XPutImage           = xlib_sym("XPutImage");
    XFlush              = xlib_sym("XFlush");
    XDestroyWindow      = xlib_sym("XDestroyWindow");
    XDestroyImage       = xlib_sym("XDestroyImage");

    g_dpy = XOpenDisplay(display);
    if (!g_dpy) { fprintf(stderr, "[WINDOW] cannot open display %s\n", display); return -1; }

    int screen = XDefaultScreen(g_dpy);
    Window root = XDefaultRootWindow(g_dpy);

    g_win = XCreateSimpleWindow(g_dpy, root, 100, 100, width, height, 1, 0, 0);
    XStoreName(g_dpy, g_win, "Parasite Eve (native port) — Phase 6A");
    XMapWindow(g_dpy, g_win);

    g_gc = XCreateGC(g_dpy, g_win, 0, NULL);

    /* Allocate image buffer: 4 bytes per pixel (X11 uses 32-bit) */
    g_imgbuf = calloc(width * height, 4);
    g_ximg = XCreateImage(g_dpy, NULL, 24, 2 /* ZPixmap */, 0,
                          (char *)g_imgbuf, width, height, 32, 0);

    XFlush(g_dpy);
    fprintf(stderr, "[WINDOW] opened %dx%d on %s\n", width, height, display);
    return 0;
}

void HostWindow_Blit(const uint8_t *rgb, int width, int height)
{
    if (!g_ximg || !g_imgbuf) return;
    /* Convert RGB 8:8:8 → X11 BGRA */
    for (int i = 0; i < width * height; i++) {
        g_imgbuf[i * 4 + 0] = rgb[i * 3 + 2];  /* B */
        g_imgbuf[i * 4 + 1] = rgb[i * 3 + 1];  /* G */
        g_imgbuf[i * 4 + 2] = rgb[i * 3 + 0];  /* R */
        g_imgbuf[i * 4 + 3] = 0;               /* A (unused) */
    }
    XPutImage(g_dpy, g_win, g_gc, g_ximg, 0, 0, 0, 0, width, height);
    XFlush(g_dpy);
}

void HostWindow_Show(int milliseconds)
{
    if (!g_dpy) return;
    XFlush(g_dpy);
    if (milliseconds > 0) {
        usleep(milliseconds * 1000);
    }
}

void HostWindow_Close(void)
{
    /* XCloseDisplay handles cleanup */
    free(g_imgbuf); g_imgbuf = NULL;
    g_ximg = NULL;
    g_gc = 0;
    g_win = 0;
    
    
    if (g_dpy) { XCloseDisplay(g_dpy); g_dpy = NULL; }
    if (g_xlib) { dlclose(g_xlib); g_xlib = NULL; }
}
