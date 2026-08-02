/*
 * Phase 6C — Native entry through translated Parasite Eve main.
 *
 * Default path: func_8001220C → full PE boot chain → black frame.
 * Direct-clear shortcut requires explicit --direct-clear-test flag.
 */

#include "psx_compat.h"
#include "host_framebuffer.h"
#include "host_window.h"
#include "stub_registry.h"
#include "game_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void func_8001220C(void);
extern int  func_8006E9A0(int);

/* ── CLI ────────────────────────────────────────────────────────────── */
static struct {
    int headless, bootstrap_disc, strict_stubs;
    const char *screenshot, *trace_path;
    int hold_ms, scale, hold_until_close, debug_overlay;
    const char *window_title;
    int direct_clear_test;
    const char *stop_after_event;
    int max_main_iterations;
} g_opts = {
    .headless = 0, .bootstrap_disc = 0, .strict_stubs = 0,
    .screenshot = NULL, .trace_path = NULL,
    .hold_ms = 0, .scale = 2, .hold_until_close = 1, .debug_overlay = 0,
    .window_title = "Parasite Eve Native Port",
    .direct_clear_test = 0, .stop_after_event = NULL, .max_main_iterations = 0,
};

static void ParseArgs(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if      (!strcmp(a, "--headless"))            g_opts.headless = 1;
        else if (!strcmp(a, "--bootstrap-disc"))       g_opts.bootstrap_disc = 1;
        else if (!strcmp(a, "--strict-stubs"))         g_opts.strict_stubs = 1;
        else if (!strcmp(a, "--windowed"))             g_opts.headless = 0;
        else if (!strcmp(a, "--hold-until-close"))     g_opts.hold_until_close = 1;
        else if (!strcmp(a, "--debug-overlay"))        g_opts.debug_overlay = 1;
        else if (!strcmp(a, "--direct-clear-test"))    g_opts.direct_clear_test = 1;
        else if (i+1<argc && !strcmp(a, "--screenshot"))      g_opts.screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--trace"))           g_opts.trace_path = argv[++i];
        else if (i+1<argc && !strcmp(a, "--hold-ms"))         g_opts.hold_ms = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--scale"))           g_opts.scale = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--window-title"))    g_opts.window_title = argv[++i];
        else if (i+1<argc && !strcmp(a, "--stop-after-event")) g_opts.stop_after_event = argv[++i];
        else if (i+1<argc && !strcmp(a, "--max-main-iterations")) g_opts.max_main_iterations = atoi(argv[++i]);
        else { fprintf(stderr, "Unknown: %s\n", a); exit(1); }
    }
    if (g_opts.hold_ms > 0) g_opts.hold_until_close = 0;
}

/* ── Trace ──────────────────────────────────────────────────────────── */
static FILE *g_trace_fp = NULL; static int g_trace_seq = 0;
static void TraceInit(void) { if (g_opts.trace_path) g_trace_fp = fopen(g_opts.trace_path, "w"); }
static void TraceEvent(const char *e) {
    if (g_trace_fp) { fprintf(g_trace_fp, "%04d %s\n", ++g_trace_seq, e); fflush(g_trace_fp); }
    fprintf(stderr, "[TRACE %04d] %s\n", g_trace_seq, e);
}
static void TraceClose(void) { if (g_trace_fp) { fclose(g_trace_fp); g_trace_fp = NULL; } }
void Trace_Direct(const char *e) { TraceEvent(e); }

/* ── Title overlay ──────────────────────────────────────────────────── */
static void UpdateTitle(const char *phase, const char *func) {
    if (!g_opts.debug_overlay || !g_host_window_open) return;
    char buf[256];
    int vs, ds, pr, mk;
    HostFB_GetState(&vs, &ds, &pr, &mk);
    snprintf(buf, sizeof(buf),
        "%s | %s | %s | iter %d | frame %d | vsync %d | stubs %d",
        g_opts.window_title, phase, func ? func : "-",
        g_port_main_iterations, pr, vs, g_stub_bootstrap_invocations);
    HostWindow_SetTitle(buf);
}

/* ── main ───────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    ParseArgs(argc, argv); TraceInit();
    g_bootstrap_disc = g_opts.bootstrap_disc;
    g_strict_stubs   = g_opts.strict_stubs;

    /* Phase 6D-S: initialize host-safe subsystems */
    PE_RamInit();
    PE_Callback_Init();
    Bootstrap_Init();
    if (g_strict_stubs) Bootstrap_EnableStrict();

    TraceEvent("native_executable_start");

    int use_window = !g_opts.headless;
    if (use_window) {
        const char *dpy = getenv("DISPLAY") ? getenv("DISPLAY") : ":10.0";
        int w = PE_PORT_FB_WIDTH * g_opts.scale;
        int h = PE_PORT_FB_HEIGHT * g_opts.scale;
        if (HostWindow_Open(dpy, w, h, g_opts.window_title, g_opts.scale) != 0)
            use_window = 0;
    }

    if (g_opts.direct_clear_test) {
        /* Explicit test path — requires --direct-clear-test flag */
        TraceEvent("direct_clear_test_begin");
        UpdateTitle("Direct clear test", "func_8006E9A0");
        func_8006E9A0(0);
        TraceEvent("direct_clear_test_done");
    } else {
        /* Normal path: translated Parasite Eve main */
        TraceEvent("call_func_8001220C");
        UpdateTitle("Main entry", "func_8001220C");
        func_8001220C();
        TraceEvent("func_8001220C_returned");
    }

    TraceEvent("shutdown_begin");

    if (use_window) {
        TraceEvent("window_blit");
        HostWindow_Blit(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        UpdateTitle("Black frame", "waiting");
        if (g_opts.hold_until_close) {
            fprintf(stderr, "[WINDOW] Open until close/Escape...\n");
            HostWindow_Run(-1);
        } else if (g_opts.hold_ms > 0) {
            HostWindow_Run(g_opts.hold_ms);
        } else {
            HostWindow_Run(2000);
        }
        HostWindow_Close();
    }

    const char *sp = g_opts.screenshot ? g_opts.screenshot : "/tmp/pe-port-black.ppm";
    if (HostFB_WritePPM(sp) == 0)
        fprintf(stderr, "[SCREENSHOT] %s (%dx%d)\n", sp, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);

    Stub_PrintSummary();
    int vs, ds, pr, mk; HostFB_GetState(&vs, &ds, &pr, &mk);
    fprintf(stderr, "[FB] vsyncs=%d drawsyncs=%d presents=%d mask=%d main_iters=%d\n",
            vs, ds, pr, mk, g_port_main_iterations);

    TraceEvent("shutdown_end"); TraceClose();
    PE_RamDestroy();
    return 0;
}
