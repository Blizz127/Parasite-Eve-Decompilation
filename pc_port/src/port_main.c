/*
 * Phase 6C — Native entry through translated Parasite Eve main.
 *
 * Default path: func_8001220C → full PE boot chain → black frame.
 * Direct-clear shortcut requires explicit --direct-clear-test flag.
 */

#include "psx_compat.h"
#include "host_framebuffer.h"
#include "host_vram.h"
#include "host_window.h"
#include "stub_registry.h"
#include "pe_bootstrap.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_disc.h"
#include "pe_cdreg.h"
#include "pe_guest_image.h"
#include "pe_route_pad.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

extern void func_8001220C(void);
extern int  func_8006E9A0(int);
extern pe_addr_t D_80011614;
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int  func_80070DD0(int, int);

/* ── CLI ────────────────────────────────────────────────────────────── */
static struct {
    int headless, bootstrap_disc, strict_stubs;
    const char *screenshot, *vram_screenshot, *vram_raw, *trace_path;
    int hold_ms, scale, hold_until_close, debug_overlay;
    const char *window_title;
    int direct_clear_test;
    const char *stop_after_event;
    int max_frames, max_main_iterations;
    const char *disc_image;
    int disc_load_test;
    int rng_oracle_dump;
    int lzcr_oracle_dump;
    int callback_oracle_dump;
    int dma_checkpoint_report;
    int skip_movie;
    int skip_opening_menu;
    int boundary_report;
    int route_pad;
    int auto_quit;
} g_opts = {
    .headless = 0, .bootstrap_disc = 0, .strict_stubs = 0,
    .screenshot = NULL, .vram_screenshot = NULL, .vram_raw = NULL,
    .trace_path = NULL,
    .hold_ms = 0, .scale = 2, .hold_until_close = 1, .debug_overlay = 0,
    .window_title = "Parasite Eve Native Port",
    .direct_clear_test = 0, .stop_after_event = NULL,
    .max_frames = 0, .max_main_iterations = 0,
    .disc_image = NULL, .disc_load_test = 0, .rng_oracle_dump = 0,
    .lzcr_oracle_dump = 0, .callback_oracle_dump = 0,
    .dma_checkpoint_report = 0,
    .skip_movie = 0, .boundary_report = 0,
    .route_pad = 0, .auto_quit = 0,
};

/* ── Deterministic route pad (interactive autopilot) ────────────────────
 * The interactive entry point has no SIO/pad, so without an installed
 * source the guest never sees a button (func_8003F3C4's idle-zero
 * normalize fills 0xFFFF) and cold boot parks at the field prefix waiting
 * for input.  This drives the SAME four-stage pad the boot -> Day-2 route
 * harness uses (pc_port/tests/test_route_boot_day2.c) via the shared
 * pc_port/include/pe_route_pad.h table, so the SAME Day-1 field route the
 * harness proves is also reachable from the windowed binary.  It is a host
 * input source only; it never writes guest state that the pad path would
 * not. */

static PeRoutePadConfig g_route_pad;
static int g_route_frame;
#define GA_TOKEN D_8009D280
static int g_frame;
static unsigned g_sewer_victories, g_sewer_enemy_peak[3];
static int g_pulse_end = 33620, g_pulse_resume = 35300;
static int g_exact_pad_begin = 42713, g_exact_pad_end = 45041;
static int g_sewer_pad_begin = 50500, g_sewer_pad_end = 51200;
static int g_second_sewer_pad_begin = 52344, g_second_sewer_pad_end = 54500;
static int g_supply_pad_begin = 54500, g_supply_pad_end = 62000;
static struct { int frame; uint16_t mask; } g_pad_sequence[4096];
static unsigned g_pad_sequence_count;
#include "route_rehearsal_pads.h"
#include "route_reward_sewer_pilot.h"

/* m0004i module-4 type-4 task PC: the documented executed-route frontier
 * (same FRONTIER_PC the harness pins).  Matched on the task PC rather than
 * the room token because the m0004i token (0xA8000248) is also live earlier
 * in the route, before the m0378i/m0377i bounce.  The frontier additionally
 * requires persist[1]==0x17A, which only the post-bounce m0004i visit holds;
 * the first m0004i visit has persist[1]==3 and also parks module 4 on the
 * same task PC. */
#define ROUTE_FRONTIER_PC       0x801B6CC8u
#define ROUTE_FRONTIER_TOKEN    0xA8000248u
#define ROUTE_FRONTIER_PERSIST1 0x0000017Au

static void RoutePadAutoQuitIfDone(void)
{
    pe_addr_t actor;
    int guard = 0;

    if (!g_opts.auto_quit) return;
    /* The m0377i module-5 transfer bounces through m0378i back to m0004i. */
    if (D_8009D280 != ROUTE_FRONTIER_TOKEN) return;
    if (PE_LoadU32(0x800A77F4u) != ROUTE_FRONTIER_PERSIST1) return;
    actor = PE_LoadU32(0x8009D20Cu);
    while (actor != 0u && guard < 64) {
        pe_addr_t task = PE_LoadU32(actor + 0xA8u);
        if (task != 0u && PE_LoadU32(task) == ROUTE_FRONTIER_PC) {
            fprintf(stderr,
                    "[ROUTE] reached m0004i frontier (pc=0x%08X) at "
                    "route frame %d; quitting (--auto-quit)\n",
                    (unsigned)ROUTE_FRONTIER_PC, g_route_frame);
            PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);
            return;
        }
        actor = PE_LoadU32(actor + 4u);
        guard++;
    }
}

static void RecordSewerVictory(void)
{
    unsigned room;
    unsigned enemies=0;
    pe_addr_t actor,aya,record;
    uint32_t flags;
    if (GA_TOKEN==0xA80023C8u) room=0;
    else if (GA_TOKEN==0xA8002448u) room=1;
    else if (GA_TOKEN==0xA8003148u) room=2;
    else return;
    actor=PE_LoadU32(0x8009D20Cu);
    for (unsigned i=0;actor && i<64u;i++,actor=PE_LoadU32(actor+4u)) {
        unsigned type=PE_LoadU8(actor+12u);
        if ((room==0?type==3u:room==1?(type==7u || type==8u):type==6u) && PE_LoadU32(actor)) enemies++;
    }
    flags=PE_LoadU32(0x8009D1A0u);
    if ((flags&2u) && enemies>g_sewer_enemy_peak[room]) g_sewer_enemy_peak[room]=enemies;
    aya=PE_LoadU32(0x8009D254u); record=aya?PE_LoadU32(aya):0u;
    if (g_sewer_enemy_peak[room]==(room==2?2u:3u) && !enemies && !(flags&6u) &&
        PE_LoadU32(0x8009D28Cu)==9u && record && PE_LoadU16(record+12u)>0u &&
        !(g_sewer_victories&(1u<<room))) {
        g_sewer_victories|=1u<<room;
        fprintf(stderr,"route: sewer victory room=%u frame=%d HP=%u\n",room+1u,g_frame,PE_LoadU16(record+12u));
    }
}

static void RoutePadLoadSequence(void)
{
    const char *s = getenv("PE_ROUTE_PAD_SEQUENCE");
    if (!s) s=kDay1RoutePads;
    g_pad_sequence_count=0;
    while (s && s[0]) {
        char *end;
        unsigned long frame, mask;
        errno=0;
        frame=strtoul(s,&end,10);
        if (errno || end==s || *end!=':' || frame>INT_MAX
            || g_pad_sequence_count==sizeof(g_pad_sequence)/sizeof(g_pad_sequence[0])
            || (g_pad_sequence_count && frame<=(unsigned)g_pad_sequence[g_pad_sequence_count-1].frame))
            break;
        s=end+1;
        errno=0;
        mask=strtoul(s,&end,16);
        if (errno || end==s || mask>0xFFFFu || (*end && *end!=',')) break;
        g_pad_sequence[g_pad_sequence_count].frame=(int)frame;
        g_pad_sequence[g_pad_sequence_count++].mask=(uint16_t)mask;
        s=*end?end+1:end;
    }
}

static uint16_t RoutePadSource(void)
{
    uint16_t mask;

    g_route_frame = g_frame;
    mask=PeRoutePad_Mask(&g_route_pad,g_frame);
    for (unsigned i=0;i<g_pad_sequence_count && g_frame>=g_pad_sequence[i].frame;i++)
        mask=g_pad_sequence[i].mask;
    if (!(g_frame>=g_exact_pad_begin && g_frame<g_exact_pad_end) &&
        !(g_frame>=g_sewer_pad_begin && g_frame<g_sewer_pad_end) &&
        !(g_frame>=g_second_sewer_pad_begin && g_frame<g_second_sewer_pad_end) &&
        !(g_frame>=g_supply_pad_begin && g_frame<g_supply_pad_end) &&
        (g_frame<g_pulse_end || g_frame>=g_pulse_resume) && (g_frame%g_route_pad.period)==3)
        mask&=g_route_pad.pulse;
    mask=RouteRewardSewerPilot(mask);
    if (getenv("PE_ROUTE_DEBUG") && (g_frame % 500) == 0)
        fprintf(stderr, "[ROUTE] padf=%d token=%08X story=%08X held=%08X pad=%04X\n",
                g_frame, (unsigned)D_8009D280,
                (unsigned)PE_LoadU32(0x800A7918u), (unsigned)PE_LoadU32(0x8009D26Cu),
                (unsigned)mask);
    RoutePadAutoQuitIfDone();
    return mask;
}

/* Phase 6E-PRS1 live-window present hook: blit the newest host pixels and
 * poll for close/Escape.  Host data only; never touches guest state. */
static void PresentHook_BlitWindow(void)
{
    if (g_opts.route_pad) {
        g_frame++;
        RecordSewerVictory();
        if ((g_frame % 500) == 0)
            fprintf(stderr, "[ROUTE] frame=%d token=%08X story=%08X victories=%u\n",
                    g_frame, (unsigned)D_8009D280,
                    (unsigned)PE_LoadU32(0x800A7918u), g_sewer_victories);
    }
    if (HostWindow_Pace()) {PE_Port_RequestStop(PE_PORT_STOP_HOST_QUIT);return;}
    HostWindow_Blit(HostFB_GetPixels(), PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);
    (void)HostWindow_Poll();
}

static int ParsePositiveLimit(const char *option, const char *value) {
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (!value[0] || !end || *end || parsed < 1 || parsed > INT_MAX) {
        fprintf(stderr, "%s requires a positive integer\n", option);
        exit(1);
    }
    return (int)parsed;
}

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
        else if (!strcmp(a, "--callback-oracle-dump")) g_opts.callback_oracle_dump = 1;
        else if (!strcmp(a, "--dma-checkpoint-report")) g_opts.dma_checkpoint_report = 1;
        else if (!strcmp(a, "--skip-movie"))           g_opts.skip_movie = 1;
        else if (!strcmp(a, "--skip-opening-menu"))    g_opts.skip_opening_menu = 1;
        else if (!strcmp(a, "--route-pad"))            g_opts.route_pad = 1;
        else if (!strcmp(a, "--auto-quit"))            g_opts.auto_quit = 1;
        else if (!strcmp(a, "--boundary-report"))      g_opts.boundary_report = 1;
        else if (i+1<argc && !strcmp(a, "--screenshot"))      g_opts.screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--vram-screenshot")) g_opts.vram_screenshot = argv[++i];
        else if (i+1<argc && !strcmp(a, "--vram-raw"))        g_opts.vram_raw = argv[++i];
        else if (i+1<argc && !strcmp(a, "--trace"))           g_opts.trace_path = argv[++i];
        else if (i+1<argc && !strcmp(a, "--hold-ms"))         g_opts.hold_ms = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--scale"))           g_opts.scale = atoi(argv[++i]);
        else if (i+1<argc && !strcmp(a, "--window-title"))    g_opts.window_title = argv[++i];
        else if (i+1<argc && !strcmp(a, "--stop-after-event")) g_opts.stop_after_event = argv[++i];
        else if (i+1<argc && !strcmp(a, "--max-frames"))
            g_opts.max_frames = ParsePositiveLimit(a, argv[++i]);
        else if (i+1<argc && !strcmp(a, "--max-main-iterations"))
            g_opts.max_main_iterations = ParsePositiveLimit(a, argv[++i]);
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
    uint32_t size, load_size, load_sectors;
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
    load_sectors = (load_size + PE_DISC_USER_SECTOR - 1u) /
                   PE_DISC_USER_SECTOR;
    load_size = load_sectors * PE_DISC_USER_SECTOR;
    snprintf(buf, sizeof(buf), "disc_load_pe_img_lba=%d_size=%u", lba, size);
    TraceEvent(buf);
    if (lba <= 0 || size == 0) { TraceEvent("disc_load_test_fail"); return 1; }

    r = func_8006E6D4(lba, 0, D_80011614, (int)load_sectors);
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

/* ── Callback oracle dump ───────────────────────────────────────────── */
/* Phase 6E-B6 verification driver (pure guest RAM + host bindings — no
 * --disc-image required): mirrors the fixed operation script of
 * pc_port/tools/callback_oracle.py op-for-op through the production
 * func_80073D24 / PE_Callback_SetSlot / PE_Callback_Dispatch path and
 * prints byte-identical lines.  The phase gate diffs the two outputs. */
static pe_addr_t  g_cb_dump_visits[16];
static int        g_cb_dump_visit_count;
static void CbDumpVisitA(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010000u; }
static void CbDumpVisitB(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010004u; }
static void CbDumpVisitC(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x80010008u; }
static void CbDumpVisitD(void) { g_cb_dump_visits[g_cb_dump_visit_count++] = 0x8003E91Cu; }

static void CbDumpWset(pe_addr_t handler) {
    uint32_t prev = func_80073D24(handler);
    printf("wset handler=0x%08X prev=0x%08X slot4=0x%08X\n",
           handler, prev, PE_LoadU32(0x8009569Cu));
}
static void CbDumpSset(uint32_t slot, pe_addr_t handler) {
    uint32_t prev = PE_Callback_SetSlot(slot, handler);
    printf("sset slot=%u handler=0x%08X prev=0x%08X\n", slot, handler, prev);
}
static void CbDumpDispatch(void) {
    g_cb_dump_visit_count = 0;
    PE_Callback_Dispatch();
    printf("dispatch counter=%u visits=",
           PE_LoadU32(0x800956ACu));
    if (g_cb_dump_visit_count == 0) {
        printf("-");
    } else {
        for (int i = 0; i < g_cb_dump_visit_count; i++) {
            printf("%s0x%08X", i ? "," : "", g_cb_dump_visits[i]);
        }
    }
    printf("\n");
}

static int RunCallbackOracleDump(void) {
    PE_Callback_Bind(0x80010000u, CbDumpVisitA);
    PE_Callback_Bind(0x80010004u, CbDumpVisitB);
    PE_Callback_Bind(0x80010008u, CbDumpVisitC);
    PE_Callback_Bind(0x8003E91Cu, CbDumpVisitD);

    CbDumpWset(0x00000000u);            /* clear slot 4                    */
    CbDumpWset(0x8003E91Cu);            /* install boot callback           */
    CbDumpWset(0x8003E91Cu);            /* repeated install: no store      */
    CbDumpWset(0x80010000u);            /* replacement                     */
    CbDumpWset(0x00000000u);            /* removal                         */
    CbDumpWset(0x00000000u);            /* repeated removal                */
    CbDumpSset(0, 0x80010000u);
    CbDumpSset(2, 0x80010004u);
    CbDumpSset(7, 0x80010008u);
    CbDumpDispatch();                   /* counter=1, visits slots 0,2,7   */
    CbDumpWset(0x8003E91Cu);            /* re-install slot 4               */
    CbDumpDispatch();                   /* counter=2, visits 0,2,4,7       */
    CbDumpSset(8, 0x11111111u);         /* slot 8 aliases counter (retail) */

    printf("snapshot slots=");
    for (uint32_t i = 0; i < 8; i++) {
        printf("%s0x%08X", i ? "," : "", PE_Callback_GetSlot(i));
    }
    printf(" counter=0x%08X\n", PE_LoadU32(0x800956ACu));
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
    HostFB_Init();
    PE_Port_RunControlReset();
    PE_Port_SetFrameLimit(g_opts.max_frames);
    PE_Port_SetMainIterationLimit(g_opts.max_main_iterations);
    PE_Port_SetSkipMovie(g_opts.skip_movie);
    PE_Port_SetSkipOpeningMenu(g_opts.skip_opening_menu);
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
        if (PE_Globals_AdoptRetailImage() != 0) {
            fprintf(stderr, "[DISC] retail overlay authority is invalid\n");
            TraceClose();
            PE_Disc_Close(disc);
            PE_RamDestroy();
            return 1;
        }
        fprintf(stderr, "[DISC] boot executable loaded into guest RAM\n");
        /* Mounted-disc command device: required for the streaming DMA path
         * (7C564 -> B89F4) that assembles a movie frame.  Stage147/148 keep
         * enable explicit for tests; production --disc-image must opt in too
         * or the E0 poll has no device to advance and never publishes a
         * populated frame.  Restored after the decomp-port refactor dropped
         * it (DAY2-158i evidence). */
        if (!PE_CdReg_EnableDevice(7u)) {
            fprintf(stderr,
                    "[DISC] CD command device already enabled or attach failed\n");
        } else {
            fprintf(stderr, "[DISC] CD command device enabled (mask=7)\n");
            TraceEvent("cd_command_device_enabled");
        }
    }

    TraceEvent("native_executable_start");

    if (g_opts.lzcr_oracle_dump) {
        int rc = RunLzcrOracleDump();
        TraceClose();
        PE_Disc_Close(disc);
        PE_RamDestroy();
        return rc;
    }

    if (g_opts.callback_oracle_dump) {
        int rc = RunCallbackOracleDump();
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
        else {
            PE_Port_SetQuitPoll(HostWindow_Poll);
            PE_Port_SetPresentHook(PresentHook_BlitWindow);
            PE_Port_SetPadSource(HostWindow_PadRaw);
        }
    }

    if (g_opts.route_pad) {
        /* Interactive Day-1 autopilot: same four-stage table + switch frames
         * the route harness proves (shared pe_route_pad.h).  A route-pad run
         * implies both documented HOST_ADAPTED skips, otherwise the
         * untranslated title/menu is still in front of the field and no pad
         * can get past it. */
        PeRoutePad_ConfigFromEnv(&g_route_pad);
        g_route_frame = 0;
        g_frame = 0;
        g_sewer_victories = 0;
        g_sewer_enemy_peak[0]=g_sewer_enemy_peak[1]=g_sewer_enemy_peak[2]=0;
        RoutePadLoadSequence();
        PE_Port_SetSkipMovie(1);
        PE_Port_SetSkipOpeningMenu(1);
        PE_Port_SetPadSource(RoutePadSource);
        fprintf(stderr,
                "[ROUTE] --route-pad: full Day-1/Day-2 pad sequence (%u pairs) "
                "+ sewer/M34 pilot + skip-movie + skip-opening-menu\n",
                g_pad_sequence_count);
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
        if (PE_Port_GetStopReason() != PE_PORT_STOP_NONE) {
            /* A quit request or explicit budget already ended execution. */
        } else if (g_opts.hold_until_close) {
            fprintf(stderr, "[WINDOW] Open until close/Escape...\n");
            HostWindow_Run(-1);
        } else if (g_opts.hold_ms > 0) {
            HostWindow_Run(g_opts.hold_ms);
        } else {
            HostWindow_Run(2000);
        }
        PE_Port_SetQuitPoll(NULL);
        PE_Port_SetPresentHook(NULL);
        HostWindow_Close();
    }

    const char *sp = g_opts.screenshot ? g_opts.screenshot : "/tmp/pe-port-black.ppm";
    if (HostFB_WritePPM(sp) == 0)
        fprintf(stderr, "[SCREENSHOT] %s (%dx%d)\n", sp, PE_PORT_FB_WIDTH, PE_PORT_FB_HEIGHT);

    if (g_opts.vram_raw) {
        if (HostVRAM_WriteRaw(g_opts.vram_raw) == 0) {
            fprintf(stderr, "[VRAM-RAW] %s (%ux%u RGB555/STP little-endian)\n",
                    g_opts.vram_raw, PE_GPU_VRAM_WIDTH, PE_GPU_VRAM_HEIGHT);
        } else {
            fprintf(stderr, "[VRAM-RAW] failed to write '%s'\n", g_opts.vram_raw);
        }
    }
    if (g_opts.vram_screenshot) {
        if (HostVRAM_WritePPM(g_opts.vram_screenshot) == 0) {
            fprintf(stderr, "[VRAM-SCREENSHOT] %s (%ux%u RGB555 diagnostic)\n",
                    g_opts.vram_screenshot,
                    PE_GPU_VRAM_WIDTH, PE_GPU_VRAM_HEIGHT);
        } else {
            fprintf(stderr, "[VRAM-SCREENSHOT] failed to write '%s'\n",
                    g_opts.vram_screenshot);
        }
    }

    Stub_PrintSummary();
    int vs, ds, pr, mk; HostFB_GetState(&vs, &ds, &pr, &mk);
    fprintf(stderr, "[FB] vsyncs=%d drawsyncs=%d presents=%d mask=%d main_iters=%d\n",
            vs, ds, pr, mk, g_port_main_iterations);
    fprintf(stderr, "[HOST] stop_reason=%s\n",
            PE_Port_StopReasonName(PE_Port_GetStopReason()));
    {
        PeGpuState gpu;

        PE_GPU_GetState(&gpu);
        fprintf(stderr,
                "[GPU] fills=%llu mono_rects=%llu tex_rects=%llu moves=%llu polygons=%llu polygon_pixels=%llu "
                "draw_mode_writes=%llu last_mono=0x%08X@0x%08X size=0x%08X\n",
                (unsigned long long)gpu.fill_count,
                (unsigned long long)gpu.mono_rectangle_count,
                (unsigned long long)gpu.rectangle_count,
                (unsigned long long)gpu.move_count,
                (unsigned long long)gpu.polygon_count,
                (unsigned long long)gpu.polygon_pixel_count,
                (unsigned long long)gpu.draw_mode_count,
                gpu.mono_rectangle_command, gpu.mono_rectangle_position,
                gpu.mono_rectangle_size);
    }
    if (g_opts.dma_checkpoint_report) {
        PEPortDmaIrqCheckpointTrace checkpoint;

        PE_Port_GetDmaIrqCheckpointTrace(&checkpoint);
        fprintf(stderr,
                "[DMA_CHECKPOINT] calls=%llu queries=%llu services=%llu "
                "captured=%llu serviced=%llu\n",
                (unsigned long long)checkpoint.checkpoint_calls,
                (unsigned long long)checkpoint.token_queries,
                (unsigned long long)checkpoint.service_calls,
                (unsigned long long)checkpoint.last_captured_token,
                (unsigned long long)checkpoint.last_serviced_token);
    }

    if (g_opts.boundary_report) {
        /* Recorded boundary/indirect-call arguments, oldest first.  This is
         * the census view of exactly which guest shape a named cut refused
         * (e.g. the GP0 word a DrawOTag node carried), so frontier work can
         * be scoped from evidence rather than guessed. */
        fprintf(stderr, "[BOUNDARY_REPORT] %d recorded arg4 call(s)%s\n",
                g_bootstrap_arg4_call_count,
                g_bootstrap_arg4_call_count >= BOOTSTRAP_MAX_ARG4_CALLS
                    ? " (log full; later calls dropped)" : "");
        for (int i = 0; i < g_bootstrap_arg4_call_count; i++) {
            const BootstrapArgCall4 *c = &g_bootstrap_arg4_calls[i];
            fprintf(stderr,
                    "[BOUNDARY_REPORT] %3d %s <- %s target=0x%08lX "
                    "a0=0x%08lX a1=0x%08lX a2=0x%08lX a3=0x%08lX",
                    i, c->symbol, c->caller,
                    (unsigned long)c->target, (unsigned long)c->arg0,
                    (unsigned long)c->arg1, (unsigned long)c->arg2,
                    (unsigned long)c->arg3);
            if (c->payload_size) {
                fprintf(stderr, " payload=");
                for (uint32_t k = 0; k < c->payload_size; k++)
                    fprintf(stderr, "%02X", c->payload[k]);
            }
            fprintf(stderr, "\n");
        }
    }

    TraceEvent("shutdown_end"); TraceClose();
    PE_Disc_Close(disc);
    PE_RamDestroy();
    return 0;
}
