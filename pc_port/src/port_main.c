/*
 * Phase 6A — Native Parasite Eve port entry point.
 *
 * Headless-first.  No emulator, no SDL2, no OpenGL.
 * Translates PE boot functions via native compilation with PS1 SDK stubs.
 */

#include "psx_compat.h"
#include "host_framebuffer.h"
#include "host_window.h"
#include "stub_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Forward declarations for translated PE functions ───────────────── */

/* Matched C — compiles natively with PS1 SDK stubs */
extern int  func_8006E9A0(int arg);   /* clear-frame function */
extern void func_8006E834(void);      /* called before clear frame */
extern void func_8001220C(void);      /* main */

/* ── CLI ─────────────────────────────────────────────────────────────── */

static struct {
    int   headless;
    int   bootstrap_disc;
    int   max_frames;
    const char *screenshot;
    const char *trace_path;
    int   strict_stubs;
    const char *disc1_path;
    const char *assets_path;
} g_opts = {
    .headless       = 0,
    .bootstrap_disc = 0,
    .max_frames     = 1,
    .screenshot     = NULL,
    .trace_path     = NULL,
    .strict_stubs   = 0,
    .disc1_path     = NULL,
    .assets_path    = NULL,
};

static void ParseArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--headless") == 0) {
            g_opts.headless = 1;
        } else if (strcmp(argv[i], "--bootstrap-disc") == 0) {
            g_opts.bootstrap_disc = 1;
        } else if (strcmp(argv[i], "--strict-stubs") == 0) {
            g_opts.strict_stubs = 1;
        } else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
            g_opts.max_frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            g_opts.screenshot = argv[++i];
        } else if (strcmp(argv[i], "--trace") == 0 && i + 1 < argc) {
            g_opts.trace_path = argv[++i];
        } else if (strcmp(argv[i], "--disc1") == 0 && i + 1 < argc) {
            g_opts.disc1_path = argv[++i];
        } else if (strcmp(argv[i], "--assets") == 0 && i + 1 < argc) {
            g_opts.assets_path = argv[++i];
        } else {
            fprintf(stderr, "Usage: %s [--headless] [--bootstrap-disc] [--strict-stubs]\n"
                    "       [--max-frames N] [--screenshot PATH] [--trace PATH]\n"
                    "       [--disc1 PATH] [--assets PATH]\n", argv[0]);
            exit(1);
        }
    }
}

/* ── Boot trace ──────────────────────────────────────────────────────── */

static FILE *g_trace_fp = NULL;
static int   g_trace_seq = 0;

static void TraceInit(void)
{
    if (g_opts.trace_path) {
        g_trace_fp = fopen(g_opts.trace_path, "w");
        if (!g_trace_fp) {
            fprintf(stderr, "WARNING: cannot open trace file '%s'\n", g_opts.trace_path);
        }
    }
}

static void TraceEvent(const char *event)
{
    if (g_trace_fp) {
        fprintf(g_trace_fp, "%04d %s\n", ++g_trace_seq, event);
        fflush(g_trace_fp);
    }
    fprintf(stderr, "[TRACE %04d] %s\n", g_trace_seq, event);
}

static void TraceClose(void)
{
    if (g_trace_fp) { fclose(g_trace_fp); g_trace_fp = NULL; }
}

/* ── Boot sequence ───────────────────────────────────────────────────── */

static int BootToBlack(void)
{
    TraceEvent("host_init_begin");
    HostFB_Init();
    TraceEvent("host_init_end");

    if (g_opts.bootstrap_disc) {
        TraceEvent("bootstrap_disc_mode");

        /* Phase 6A bootstrap path:
         *   func_8006E834()  → boot setup (BOOTSTRAP_RET stubs inside)
         *   func_8006E9A0(0) → clear frame (real matched C)
         *
         * The clear frame function calls:
         *   VSync(0)         → HostFB_VSync (IMPLEMENTED)
         *   SetDispMask(0)   → HostFB_SetDispMask (IMPLEMENTED)
         *   PutDispEnv(...)  → HostFB_Present (HOST_ADAPTED)
         *   ClearImage(...)  → HostFB_ClearImage (IMPLEMENTED)
         *   DrawSync(0)      → HostFB_DrawSync (IMPLEMENTED)
         */

        TraceEvent("call_func_8006E834");
        func_8006E834();  /* BOOTSTRAP_RET stubs for boot setup */

        TraceEvent("call_func_8006E9A0");
        int ret = func_8006E9A0(0);  /* REAL matched C — the clear-frame function */
        TraceEvent("func_8006E9A0_returned");
        (void)ret;

        TraceEvent("first_frame_presented");
    } else {
        /* Future: real disc mode calls func_8001220C → full boot chain */
        TraceEvent("real_disc_boot_begin");
        fprintf(stderr, "Real disc mode not yet implemented. Use --bootstrap-disc.\n");
        return 1;
    }

    TraceEvent("boot_complete");
    return 0;
}

/* ── main ────────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    ParseArgs(argc, argv);
    TraceInit();

    g_bootstrap_disc = g_opts.bootstrap_disc;
    g_strict_stubs   = g_opts.strict_stubs;

    TraceEvent("native_executable_start");

    /* Open a window on the desktop (unless --headless) */
    int use_window = !g_opts.headless;
    if (use_window) {
        const char *dpy = getenv("DISPLAY");
        if (!dpy) dpy = ":10.0";
        if (HostWindow_Open(dpy, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT) != 0) {
            fprintf(stderr, "[WINDOW] Falling back to headless mode\n");
            use_window = 0;
        }
    }

    int result = BootToBlack();

    TraceEvent("shutdown_begin");

    /* Blit framebuffer to window so the user can SEE it */
    if (use_window) {
        TraceEvent("window_blit");
        HostWindow_Blit(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        fprintf(stderr, "[WINDOW] Black frame shown — 4 seconds...\n");
        HostWindow_Show(4000);
    }

    /* Screenshot output */
    const char *screenshot_path = g_opts.screenshot ? g_opts.screenshot : "/tmp/pe-port-black.ppm";
    if (HostFB_WritePPM(screenshot_path) == 0) {
        fprintf(stderr, "[SCREENSHOT] %s (%dx%d PPM)\n", screenshot_path,
                PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
        TraceEvent("screenshot_written");
    } else {
        fprintf(stderr, "[ERROR] Failed to write screenshot to %s\n", screenshot_path);
    }

    /* Stub summary */
    Stub_PrintSummary();

    /* Framebuffer state */
    int vs, ds, pr, mk;
    HostFB_GetState(&vs, &ds, &pr, &mk);
    fprintf(stderr, "[FRAMEBUFFER] vsyncs=%d drawsyncs=%d presents=%d mask=%d\n",
            vs, ds, pr, mk);

    if (use_window) {
        TraceEvent("window_close");
        HostWindow_Close();
    }

    TraceEvent("shutdown_end");
    TraceClose();

    return result;
}
