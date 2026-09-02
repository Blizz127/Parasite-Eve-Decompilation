/*
 * Phase 6B — X11 window via dlopen with event loop.
 *
 * Key symbols we need (XK_Escape = 9):
 * We don't include X11/keysymdef.h, so define the one we need.
 */
#include "host_window.h"
#include "host_framebuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>

typedef unsigned long KeySym;
#define XK_Escape 0xFF1B
#define XK_Return 0xFF0D
#define XK_space  0x0020
#define XK_Left   0xFF51
#define XK_Up     0xFF52
#define XK_Right  0xFF53
#define XK_Down   0xFF54
#define XK_z      0x007A
#define XK_x      0x0078
#define XK_Z      0x005A
#define XK_X      0x0058
#define XK_s      0x0073
#define XK_S      0x0053
#define KeyPress    2
#define KeyRelease  3
#define ButtonPress 4
#define Expose     12
#define ClientMessage 33
#define NoEventMask 0
#define KeyPressMask    (1L<<0)
#define ExposureMask    (1L<<15)
#define StructureNotifyMask (1L<<17)
#define SubstructureNotifyMask (1L<<19)

/* ── Xlib types ─────────────────────────────────────────────────────── */
typedef unsigned long XID;
typedef XID Window;
typedef XID Drawable;
typedef XID GC;
typedef unsigned long Atom;
typedef unsigned long Time;

typedef struct { int type; unsigned long serial; int send_event; void *display; Window window; } XAnyEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 Window root, subwindow; Time time; int x, y, x_root, y_root;
                 unsigned state; unsigned keycode; int same_screen; } XKeyEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 int x, y, width, height, count; } XExposeEvent;
typedef struct { int type; unsigned long serial; int send_event; void *display; Window window;
                 Atom message_type; int format; long data[5]; } XClientMessageEvent;
typedef union { int type; XAnyEvent xany; XKeyEvent xkey; XExposeEvent xexpose; XClientMessageEvent xclient; long pad[24]; } XEvent;

typedef struct { void *ext_data; int depth, bits_per_pixel, scanline_pad; } XImage;

/* ── Function pointers from libX11 ──────────────────────────────────── */
static void *g_xlib = NULL;
static void *(*XOpenDisplay)(const char *);
static int   (*XCloseDisplay)(void *);
static int   (*XDefaultScreen)(void *);
static Window (*XDefaultRootWindow)(void *);
static Window (*XCreateSimpleWindow)(void *, Window, int, int, unsigned, unsigned, unsigned, unsigned, unsigned);
static int   (*XMapWindow)(void *, Window);
static int   (*XStoreName)(void *, Window, const char *);
static GC     (*XCreateGC)(void *, Drawable, unsigned long, void *);
static int   (*XSelectInput)(void *, Window, long);
static int   (*XNextEvent)(void *, XEvent *);
static int   (*XPending)(void *);
static KeySym (*XLookupKeysym)(XKeyEvent *, int);
static XImage *(*XCreateImage)(void *, void *, unsigned, int, int, char *, unsigned, unsigned, int, int);
static int   (*XPutImage)(void *, Drawable, GC, XImage *, int, int, int, int, unsigned, unsigned);
static int   (*XFlush)(void *);
static int   (*XDestroyWindow)(void *, Window);
static Atom  (*XInternAtom)(void *, const char *, int);
static int   (*XSetWMProtocols)(void *, Window, Atom *, int);

static void *g_dpy = NULL;
static Window g_win = 0;
static GC g_gc = 0;
static XImage *g_ximg = NULL;
static uint8_t *g_imgbuf = NULL;
static int g_win_w = 0, g_win_h = 0;
static int g_fb_w = 0, g_fb_h = 0;
static Atom g_wm_delete = 0;
static int g_close_requested = 0;

int g_host_window_open = 0;
static uint16_t g_sony_held = 0;

static void *xlib_sym(const char *name) {
    void *p = dlsym(g_xlib, name);
    if (!p) { fprintf(stderr, "[WINDOW] missing symbol: %s\n", name); exit(1); }
    return p;
}

static void bind_all(void) {
    XOpenDisplay        = xlib_sym("XOpenDisplay");
    XCloseDisplay       = xlib_sym("XCloseDisplay");
    XDefaultScreen      = xlib_sym("XDefaultScreen");
    XDefaultRootWindow  = xlib_sym("XDefaultRootWindow");
    XCreateSimpleWindow = xlib_sym("XCreateSimpleWindow");
    XMapWindow          = xlib_sym("XMapWindow");
    XStoreName          = xlib_sym("XStoreName");
    XCreateGC           = xlib_sym("XCreateGC");
    XSelectInput        = xlib_sym("XSelectInput");
    XNextEvent          = xlib_sym("XNextEvent");
    XPending            = xlib_sym("XPending");
    XLookupKeysym       = xlib_sym("XLookupKeysym");
    XCreateImage        = xlib_sym("XCreateImage");
    XPutImage           = xlib_sym("XPutImage");
    XFlush              = xlib_sym("XFlush");
    XDestroyWindow      = xlib_sym("XDestroyWindow");
    XInternAtom         = xlib_sym("XInternAtom");
    XSetWMProtocols     = xlib_sym("XSetWMProtocols");
}

int HostWindow_Open(const char *display, int width, int height,
                    const char *title, int scale)
{
    g_xlib = dlopen("libX11.so.6", RTLD_LAZY);
    if (!g_xlib) { fprintf(stderr, "[WINDOW] libX11.so.6 not found\n"); return -1; }
    bind_all();

    g_dpy = XOpenDisplay(display);
    if (!g_dpy) { fprintf(stderr, "[WINDOW] cannot open display %s\n", display); return -1; }

    int screen = XDefaultScreen(g_dpy);
    Window root = XDefaultRootWindow(g_dpy);
    /* Use scale to determine visible size: framebuffer is 320×240, scale × that */
    g_fb_w = PE_PORT_FB_WIDTH;
    g_fb_h = PE_PORT_FB_HEIGHT;
    g_win_w = width > 0 ? width : g_fb_w * scale;
    g_win_h = height > 0 ? height : g_fb_h * scale;

    g_win = XCreateSimpleWindow(g_dpy, root, 200, 200, g_win_w, g_win_h,
                                 4, 0xFFFFFF, 0xFFFFFF);
    XStoreName(g_dpy, g_win, title);
    XSelectInput(g_dpy, g_win, ExposureMask | KeyPressMask | (1L<<1) | StructureNotifyMask);
    g_wm_delete = XInternAtom(g_dpy, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(g_dpy, g_win, &g_wm_delete, 1);
    XMapWindow(g_dpy, g_win);

    g_gc = XCreateGC(g_dpy, g_win, 0, NULL);
    g_imgbuf = calloc(g_win_w * g_win_h, 4);
    g_ximg = XCreateImage(g_dpy, NULL, 24, 2, 0, (char *)g_imgbuf, g_win_w, g_win_h, 32, 0);

    g_close_requested = 0;
    g_host_window_open = 1;

    XFlush(g_dpy);
    fprintf(stderr, "[WINDOW] %dx%d (fb %dx%d) on %s title='%s'\n",
            g_win_w, g_win_h, g_fb_w, g_fb_h, display, title);
    return 0;
}

void HostWindow_Blit(const uint8_t *rgb, int fb_w, int fb_h)
{
    if (!g_ximg || !g_imgbuf) return;
    int scale_x = g_win_w / fb_w;
    int scale_y = g_win_h / fb_h;
    if (scale_x < 1) scale_x = 1;
    if (scale_y < 1) scale_y = 1;
    for (int oy = 0; oy < g_win_h; oy++) {
        int sy = oy / scale_y;
        if (sy >= fb_h) sy = fb_h - 1;
        uint8_t *dst = g_imgbuf + oy * g_win_w * 4;
        for (int ox = 0; ox < g_win_w; ox++) {
            int sx = ox / scale_x;
            if (sx >= fb_w) sx = fb_w - 1;
            const uint8_t *src = rgb + (sy * fb_w + sx) * 3;
            *dst++ = src[2]; *dst++ = src[1]; *dst++ = src[0]; *dst++ = 0;
        }
    }
    XPutImage(g_dpy, g_win, g_gc, g_ximg, 0, 0, 0, 0, g_win_w, g_win_h);
    XFlush(g_dpy);
}

void HostWindow_SetTitle(const char *title)
{
    if (g_dpy && g_win) { XStoreName(g_dpy, g_win, title); XFlush(g_dpy); }
}

int HostWindow_Poll(void)
{
    if (!g_dpy) return 0;
    while (XPending(g_dpy)) {
        XEvent ev;
        XNextEvent(g_dpy, &ev);
        if (ev.type == Expose) {
            /* Redraw: blit current framebuffer again (caller does this) */
        } else if (ev.type == KeyPress || ev.type == KeyRelease) {
            KeySym ks = XLookupKeysym(&ev.xkey, 0);
            uint16_t bit = 0;
            if (ks == XK_Escape) { g_close_requested = 1; return 1; }
            if (ks == XK_space || ks == XK_z ||
                ks == XK_x || ks == XK_Z || ks == XK_X)
                bit = 0x4000u;
            else if (ks == XK_Return || ks == XK_s || ks == XK_S)
                bit = 0x0008u;          /* Start */
            else if (ks == XK_Up)
                bit = 0x0010u;
            else if (ks == XK_Right)
                bit = 0x0020u;
            else if (ks == XK_Down)
                bit = 0x0040u;
            else if (ks == XK_Left)
                bit = 0x0080u;
            if (bit != 0u) {
                if (ev.type == KeyPress)
                    g_sony_held |= bit;
                else
                    g_sony_held &= (uint16_t)~bit;
            }
        } else if (ev.type == ClientMessage) {
            if ((Atom)ev.xclient.data[0] == g_wm_delete) { g_close_requested = 1; return 1; }
        }
    }
    return g_close_requested ? 1 : 0;
}

uint16_t HostWindow_PadRaw(void)
{
    return (uint16_t)(~g_sony_held);
}

int HostWindow_Run(int milliseconds)
{
    if (!g_dpy) return 0;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        int rc = HostWindow_Poll();
        if (rc) return rc;

        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;
        if (milliseconds > 0 && elapsed >= milliseconds) return 0;

        usleep(16000); /* ~60 Hz poll */
    }
}

void HostWindow_Close(void)
{
    free(g_imgbuf); g_imgbuf = NULL; g_ximg = NULL;
    g_gc = 0; g_win = 0;
    if (g_dpy) { XCloseDisplay(g_dpy); g_dpy = NULL; }
    if (g_xlib) { dlclose(g_xlib); g_xlib = NULL; }
    g_host_window_open = 0;
    g_sony_held = 0;
}
