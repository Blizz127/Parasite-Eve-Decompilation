/*
 * Phase 6B — Native Parasite Eve port with visible X11 window.
 *
 * Default: windowed on DISPLAY, stays open until Escape or close.
 * Headless: --headless flag.
 */

#include "psx_compat.h"
#include "host_framebuffer.h"
#include "host_window.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int  func_8006E9A0(int arg);
extern void func_8006E834(void);

/* ── CLI ────────────────────────────────────────────────────────────── */

static struct {
    int headless, bootstrap_disc, strict_stubs;
    const char *screenshot, *trace_path;
    int hold_ms, scale, hold_until_close, debug_overlay;
    const char *window_title;
} g_opts = {
    .headless = 0, .bootstrap_disc = 0, .strict_stubs = 0,
    .screenshot = NULL, .trace_path = NULL,
    .hold_ms = 0, .scale = 2, .hold_until_close = 1, .debug_overlay = 0,
    .window_title = "Parasite Eve Native Port",
};

static void ParseArgs(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "--headless"))        g_opts.headless = 1;
        else if (!strcmp(a, "--bootstrap-disc"))   g_opts.bootstrap_disc = 1;
        else if (!strcmp(a, "--strict-stubs"))     g_opts.strict_stubs = 1;
        else if (!strcmp(a, "--windowed"))         g_opts.headless = 0;
        else if (!strcmp(a, "--hold-until-close")) g_opts.hold_until_close = 1;
        else if (!strcmp(a, "--debug-overlay"))    g_opts.debug_overlay = 1;
        else if (i+1<argc && !strcmp(a, "--screenshot"))  g_opts.screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--trace"))       g_opts.trace_path = argv[++i];
        else if (i+1<argc && !strcmp(a, "--hold-ms"))     g_opts.hold_ms = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--scale"))       g_opts.scale = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--window-title")) g_opts.window_title = argv[++i];
        else { fprintf(stderr, "Unknown: %s\n", a); exit(1); }
    }
    if (g_opts.hold_ms > 0) g_opts.hold_until_close = 0;
}

/* ── Boot trace ─────────────────────────────────────────────────────── */
static FILE *g_trace_fp = NULL; static int g_trace_seq = 0;
static void TraceInit(void) { if (g_opts.trace_path) g_trace_fp = fopen(g_opts.trace_path, "w"); }
static void TraceEvent(const char *e) {
    if (g_trace_fp) { fprintf(g_trace_fp, "%04d %s\n", ++g_trace_seq, e); fflush(g_trace_fp); }
    fprintf(stderr, "[TRACE %04d] %s\n", g_trace_seq, e);
}
static void TraceClose(void) { if (g_trace_fp) { fclose(g_trace_fp); g_trace_fp = NULL; } }

/* ── Title overlay update ───────────────────────────────────────────── */
static void UpdateTitle(const char *phase, const char *func) {
    if (!g_opts.debug_overlay || !g_host_window_open) return;
    char buf[256];
    int vs, ds, pr, mk;
    HostFB_GetState(&vs, &ds, &pr, &mk);
    snprintf(buf, sizeof(buf),
        "%s | %s | %s | frame %d | vsync %d | dsync %d | stubs %d",
        g_opts.window_title, phase, func ? func : "-", pr, vs, ds, g_stub_bootstrap_invocations);
    HostWindow_SetTitle(buf);
}

/* ── Boot ───────────────────────────────────────────────────────────── */
static int BootToBlack(void) {
    TraceEvent("host_init_begin");
    HostFB_Init();
    TraceEvent("host_init_end");
    if (!g_opts.bootstrap_disc) {
        fprintf(stderr, "Use --bootstrap-disc\n"); return 1;
    }
    TraceEvent("bootstrap_disc_mode");
    UpdateTitle("Native boot", "func_8006E834");
    TraceEvent("call_func_8006E834"); func_8006E834();
    UpdateTitle("Display init", "func_8006E9A0");
    TraceEvent("call_func_8006E9A0"); func_8006E9A0(0);
    TraceEvent("func_8006E9A0_returned");
    UpdateTitle("Clear frame reached", "done");
    TraceEvent("first_frame_presented");
    TraceEvent("boot_complete");
    return 0;
}

/* ── main ───────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    ParseArgs(argc, argv); TraceInit();
    g_bootstrap_disc = g_opts.bootstrap_disc;
    g_strict_stubs   = g_opts.strict_stubs;
    TraceEvent("native_executable_start");

    int use_window = !g_opts.headless;
    if (use_window) {
        const char *dpy = getenv("DISPLAY") ? getenv("DISPLAY") : ":10.0";
        int w = PE_PORT_FB_WIDTH * g_opts.scale;
        int h = PE_PORT_FB_HEIGHT * g_opts.scale;
        if (HostWindow_Open(dpy, w, h, g_opts.window_title, g_opts.scale) != 0) {
            fprintf(stderr, "[WINDOW] fallback to headless\n"); use_window = 0;
        }
    }

    int result = BootToBlack();
    TraceEvent("shutdown_begin");

    if (use_window) {
        TraceEvent("window_blit");
        HostWindow_Blit(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        UpdateTitle("Black frame", "waiting");
        if (g_opts.hold_until_close) {
            fprintf(stderr, "[WINDOW] Open until close/Escape...\n");
            HostWindow_Run(-1);
        } else if (g_opts.hold_ms > 0) {
            fprintf(stderr, "[WINDOW] Holding %d ms...\n", g_opts.hold_ms);
            HostWindow_Run(g_opts.hold_ms);
        } else {
            HostWindow_Run(2000);
        }
        TraceEvent("window_close");
        HostWindow_Close();
    }

    const char *sp = g_opts.screenshot ? g_opts.screenshot : "/tmp/pe-port-black.ppm";
    if (HostFB_WritePPM(sp) == 0) {
        fprintf(stderr, "[SCREENSHOT] %s (%dx%d)\n", sp, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        TraceEvent("screenshot_written");
    }
    Stub_PrintSummary();
    int vs, ds, pr, mk; HostFB_GetState(&vs, &ds, &pr, &mk);
    fprintf(stderr, "[FB] vsyncs=%d drawsyncs=%d presents=%d mask=%d\n", vs, ds, pr, mk);

    TraceEvent("shutdown_end"); TraceClose();
    return result;
}
