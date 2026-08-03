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
#include "pe_sdk.h"
#include "pe_disc.h"
#include "pe_guest_image.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern void func_8001220C(void);
extern int  func_8006E9A0(int);
extern pe_addr_t D_80011614;
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int  func_80070DD0(int, int);

/* ── CLI ────────────────────────────────────────────────────────────── */
static struct {
    int headless, bootstrap_disc, strict_stubs;
    const char *screenshot, *trace_path;
    int hold_ms, scale, hold_until_close, debug_overlay;
    const char *window_title;
    int direct_clear_test;
    const char *stop_after_event;
    int max_main_iterations;
    const char *disc_image;
    int disc_load_test;
    int rng_oracle_dump;
    int lzcr_oracle_dump;
} g_opts = {
    .headless = 0, .bootstrap_disc = 0, .strict_stubs = 0,
    .screenshot = NULL, .trace_path = NULL,
    .hold_ms = 0, .scale = 2, .hold_until_close = 1, .debug_overlay = 0,
    .window_title = "Parasite Eve Native Port",
    .direct_clear_test = 0, .stop_after_event = NULL, .max_main_iterations = 0,
    .disc_image = NULL, .disc_load_test = 0, .rng_oracle_dump = 0,
    .lzcr_oracle_dump = 0,
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
        else if (!strcmp(a, "--disc-load-test"))       g_opts.disc_load_test = 1;
        else if (!strcmp(a, "--rng-oracle-dump"))      g_opts.rng_oracle_dump = 1;
        else if (!strcmp(a, "--lzcr-oracle-dump"))     g_opts.lzcr_oracle_dump = 1;
        else if (i+1<argc && !strcmp(a, "--screenshot"))      g_opts.screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--trace"))           g_opts.trace_path = argv[++i];
        else if (i+1<argc && !strcmp(a, "--hold-ms"))         g_opts.hold_ms = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--scale"))           g_opts.scale = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--window-title"))    g_opts.window_title = argv[++i];
        else if (i+1<argc && !strcmp(a, "--stop-after-event")) g_opts.stop_after_event = argv[++i];
        else if (i+1<argc && !strcmp(a, "--max-main-iterations")) g_opts.max_main_iterations = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--disc-image"))      g_opts.disc_image = argv[++i];
        else { fprintf(stderr, "Unknown: %s\n", a); exit(1); }
    }
    if (g_opts.hold_ms > 0) g_opts.hold_until_close = 0;
    if (g_opts.bootstrap_disc && g_opts.disc_image) {
        fprintf(stderr, "--bootstrap-disc and --disc-image are mutually exclusive\n");
        exit(1);
    }
    if (g_opts.disc_load_test && !g_opts.disc_image) {
        fprintf(stderr, "--disc-load-test requires --disc-image\n");
        exit(1);
    }
    if (g_opts.rng_oracle_dump && !g_opts.disc_image) {
        fprintf(stderr, "--rng-oracle-dump requires --disc-image\n");
        exit(1);
    }
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

/* ── Real-disc load test ────────────────────────────────────────────── */
/* Explicit verification driver (like --direct-clear-test): runs the real
 * Phase 6E-A disc byte path against --disc-image — PVD verify, DsSearchFile
 * "\PE.IMG;1", CdPosToInt, bounded guest-RAM load at the D_80011614
 * destination, poll — and traces every value.  Deterministic for a given
 * image; used for the three-run real-disc trace gate. */
#define PE_LOADTEST_CDLFILE  0x801FFEC0u   /* documented guest scratch */
#define PE_LOADTEST_MAX      0x8000u       /* 32 KiB load cap          */

static uint64_t PeFnv1a64(const uint8_t *p, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    size_t i;
    for (i = 0; i < n; i++) { h ^= p[i]; h *= 1099511628211ULL; }
    return h;
}

static int RunDiscLoadTest(void) {
    char buf[160];
    int r;
    int lba;
    uint32_t size, load_size;
    uint64_t h;

    TraceEvent("disc_load_test_begin");
    /* Drive reset first, as the retail boot path does (CdInit/reset bring
     * the synchronous drive model to the idle lane before any verify). */
    func_8007EC14();
    func_8007ED58();
    r = func_80082314();
    snprintf(buf, sizeof(buf), "disc_load_pvd_verify=%d", r);
    TraceEvent(buf);
    if (r != 4) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_80081414(PE_LOADTEST_CDLFILE, "\\PE.IMG;1");
    snprintf(buf, sizeof(buf), "disc_load_search_pe_img=%d", r);
    TraceEvent(buf);
    if (r != 1) { TraceEvent("disc_load_test_fail"); return 1; }

    lba = func_80080C48(PE_LOADTEST_CDLFILE);
    size = PE_LoadU32(PE_LOADTEST_CDLFILE + 4);
    load_size = size < PE_LOADTEST_MAX ? size : PE_LOADTEST_MAX;
    snprintf(buf, sizeof(buf), "disc_load_pe_img_lba=%d_size=%u", lba, size);
    TraceEvent(buf);
    if (lba <= 0 || size == 0) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_8006E6D4(lba, 0, D_80011614, (int)load_size);
    snprintf(buf, sizeof(buf), "disc_load_issue=%d_dest=0x%08X_len=%u",
             r, (unsigned)D_80011614, load_size);
    TraceEvent(buf);
    if (r != 1) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_800811E4(PE_LOADTEST_CDLFILE);
    snprintf(buf, sizeof(buf), "disc_load_poll=%d", r);
    TraceEvent(buf);
    if (r != 0) { TraceEvent("disc_load_test_fail"); return 1; }

    h = PeFnv1a64(PE_Translate(D_80011614, load_size), load_size);
    snprintf(buf, sizeof(buf), "disc_load_fnv1a64=%016llX",
             (unsigned long long)h);
    TraceEvent(buf);
    TraceEvent("disc_load_test_ok");
    return 0;
}

/* ── RNG oracle dump ────────────────────────────────────────────────── */
/* Phase 6E-B2 verification driver (requires --disc-image so the retail
 * exe bytes are in guest RAM): seeds via func_80070D10, runs the 2000-call
 * warm-up, and prints checkpoints + func_80070DD0 samples to stdout in
 * exactly the format produced by pc_port/tools/rng_oracle.py.  The phase
 * gate diffs the two outputs; they must be identical. */
#define GA_DUMP_INDEX1  0x80070E04u
#define GA_DUMP_INDEX2  0x80070E08u
#define GA_DUMP_TABLE   0x80070E0Cu

static int RunRngOracleDump(void) {
    static const int k_checkpoints[] = { 1, 2, 16, 17, 64, 256, 2000 };
    static const int k_ranges[][2] = {
        { 0, 100 }, { 1, 4 }, { 0, 65536 }, { 5, 5 }, { 10, 0 }, { -3, 3 }
    };
    func_80070D10();
    for (int call = 1; call <= 2000; call++) {
        int pre1 = (int)PE_LoadU32(GA_DUMP_INDEX1);
        int pre2 = (int)PE_LoadU32(GA_DUMP_INDEX2);
        unsigned int v0 = func_80070D6C();
        for (size_t k = 0; k < sizeof(k_checkpoints)/sizeof(k_checkpoints[0]); k++) {
            if (call == k_checkpoints[k]) {
                printf("checkpoint call=%5d v0=0x%08X i1=%4d i2=%4d "
                       "addr1=0x%08X addr2=0x%08X\n",
                       call, v0,
                       (int)PE_LoadU32(GA_DUMP_INDEX1),
                       (int)PE_LoadU32(GA_DUMP_INDEX2),
                       GA_DUMP_TABLE + (pe_addr_t)pre1,
                       GA_DUMP_TABLE + (pe_addr_t)pre2);
            }
        }
    }
    for (size_t k = 0; k < sizeof(k_ranges)/sizeof(k_ranges[0]); k++) {
        int a0 = k_ranges[k][0], a1 = k_ranges[k][1];
        int out = func_80070DD0(a0, a1);
        printf("70DD0(%d,%d) = %u (0x%08X)\n", a0, a1,
               (unsigned int)out, (unsigned int)out);
    }
    return 0;
}

/* ── LZCR oracle dump ───────────────────────────────────────────────── */
/* Phase 6E-B4 verification driver (pure arithmetic + guest RAM — no
 * --disc-image required): calls the REAL func_8003EAC8 for the oracle
 * input set and prints one line per input in exactly the format produced
 * by pc_port/tools/lzcr_oracle.py (a tiny MIPS interpreter over the
 * verified retail words).  `lzcr`/`idx` come from PE_GTE_LZCR and the
 * translated branch; `dest` is found by SCANNING guest RAM for the unique
 * sentinel — independent of the formula — so an inconsistent store path
 * cannot hide.  The phase gate diffs the two outputs; must be identical. */
#define LZCR_DUMP_SCAN_LO  0x800A76E0u   /* 3 words below D_800A76F0-4   */
#define LZCR_DUMP_SCAN_HI  0x800A7770u   /* 1 word past D_800A76F0+0x7C  */

static int RunLzcrOracleDump(void) {
    static const uint32_t k_first[] = {
        0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
        0x00000008u, 0x00008000u, 0x40000000u, 0x7FFFFFFFu,
        0x80000000u, 0x80000001u, 0xC0000000u, 0xFFFFFFFFu
    };
    static const uint32_t k_skip[] = {
        0x00000001u, 0x00000002u, 0x00000008u,
        0x00008000u, 0x40000000u, 0x80000000u
    };
    uint32_t inputs[12 + 26];
    int n = 0;
    for (size_t i = 0; i < sizeof(k_first)/sizeof(k_first[0]); i++)
        inputs[n++] = k_first[i];
    for (int k = 0; k < 32; k++) {
        uint32_t m = 1u << k, skip = 0;
        for (size_t j = 0; j < sizeof(k_skip)/sizeof(k_skip[0]); j++)
            if (k_skip[j] == m) skip = 1;
        if (!skip) inputs[n++] = m;
    }

    for (int i = 0; i < n; i++) {
        uint32_t mask = inputs[i];
        uint32_t sentinel = 0xEAC80000u | (uint32_t)i;
        pe_addr_t dest = 0;
        int found = 0;
        for (pe_addr_t a = LZCR_DUMP_SCAN_LO; a <= LZCR_DUMP_SCAN_HI; a += 4)
            PE_StoreU32(a, 0);
        func_8003EAC8((int)mask, (int)sentinel);
        for (pe_addr_t a = LZCR_DUMP_SCAN_LO; a <= LZCR_DUMP_SCAN_HI; a += 4) {
            if (PE_LoadU32(a) == sentinel) { dest = a; found++; }
        }
        if (found != 1) {
            printf("EAC8 a0=0x%08X FATAL sentinel found %d times\n", mask, found);
            return 1;
        }
        int32_t idx = (mask == 0x80000000u) ? 31
                                            : 31 - (int32_t)PE_GTE_LZCR(mask);
        printf("EAC8 a0=0x%08X lzcr=%u idx=%d dest=0x%08X stored=0x%08X\n",
               mask, PE_GTE_LZCR(mask), idx, dest, PE_LoadU32(dest));
    }
    return 0;
}

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

    /* Phase 6E-A: real Disc 1 image, read-only (never copied or staged). */
    PE_Disc *disc = NULL;
    if (g_opts.disc_image) {
        char err[256];
        disc = PE_Disc_Open(g_opts.disc_image, err, sizeof(err));
        if (!disc) {
            fprintf(stderr, "[DISC] failed to open disc image: %s\n", err);
            TraceClose();
            PE_RamDestroy();
            return 1;
        }
        PE_Disc_SetActive(disc);
        fprintf(stderr, "[DISC] opened '%s' (%u user sectors)\n",
                g_opts.disc_image, PE_Disc_UserSectorCount(disc));

        /* Phase 6E-B2: load the retail boot executable into guest RAM —
         * retail code reads its own text as data (the func_80070D6C RNG
         * read cursor cycles through 14 code words below its table).
         * A disc that cannot supply its boot exe is rejected here. */
        if (PE_GuestImage_LoadExe(disc, err, sizeof(err)) != 0) {
            fprintf(stderr, "[DISC] boot executable load failed: %s\n", err);
            TraceClose();
            PE_Disc_Close(disc);
            PE_RamDestroy();
            return 1;
        }
        fprintf(stderr, "[DISC] boot executable loaded into guest RAM\n");
    }

    TraceEvent("native_executable_start");

    if (g_opts.lzcr_oracle_dump) {
        int rc = RunLzcrOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.rng_oracle_dump) {
        int rc = RunRngOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.disc_load_test) {
        int rc = RunDiscLoadTest();
        TraceEvent("shutdown_end"); TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

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
    PE_Disc_Close(disc);
    PE_RamDestroy();
    return 0;
}
