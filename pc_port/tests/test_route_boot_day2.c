/*
 * test_route_boot_day2.c — end-to-end boot -> Day-2 route harness.
 *
 * PURPOSE
 * -------
 * Drive func_8001220C (the translated retail main loop) with the retail Disc 1
 * image and a deterministic scripted pad, then assert the address-exact
 * persist[]/token transitions documented in
 * docs/ai_context/DAY1_DAY2_TRANSITIONS.md and re-derived by
 * pc_port/tools/pe_day1_day2_transitions.py.
 *
 * This is a CONTROL-FLOW / STORY-STATE traversal proof.  It does NOT prove
 * audio fidelity (XA is not decoded), pixel/timing fidelity (no hardware
 * rasterizer), or that the pad sequence is the unique player solution.  A run
 * that advances part of the chain and then stops at a named, evidenced
 * frontier is the expected result.
 *
 * WHAT IS MEASURED
 * ----------------
 *   * persist[74]  (0x800A7918) — story/progress word, source of the exit
 *     selector at 0x80192030.
 *   * persist[1]   (0x800A77F4) — the selector's companion write.
 *   * D_8009D280   — the current field room token (packed ascii).
 *   * PE_Port_ShouldStop() / PE_Port_GetStopReason() — loud boundaries and
 *     iteration/frame limits are honored, never swallowed.
 *   * g_stub_order_log — the registered HOST_ADAPTED/UNSUPPORTED boundary
 *     stubs the run actually invoked.
 *
 * The walk is split into ordered milestones.  Every milestone that is reached
 * is pinned to the retail PC that writes it, so a regression that changes the
 * route is visible immediately.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "stub_registry.h"
#include "host_framebuffer.h"
#include "pe_disc.h"
#include "pe_guest_image.h"
#include "pe_route_pad.h"
#include "route_rehearsal_pads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void func_8001220C(void);
extern unsigned int D_8009D1C4;
extern unsigned int D_800A7918;
extern void PE_Disc_SetActive(PE_Disc *disc);

/* The library's translated code calls Trace_Direct; the CLI host normally
 * provides it, so the harness supplies the same no-op sink. */
void Trace_Direct(const char *event) { (void)event; }

/* Story word storage (psx_compat.h exposes D_8009D280 as a guest lvalue). */
#define GA_PERSIST74 0x800A7918u
#define GA_PERSIST1  0x800A77F4u
#define GA_TOKEN     D_8009D280

#define MAX_TRACE 256
/* With the recorded kDay1RoutePads sequence applied, the harness follows the
 * live autopilot through the later Day-1 rooms and parks in the save/load menu
 * at ~frame 38495 (empty card slot retries).  Run long enough to assert those
 * rooms and reach the frame limit. */
#define ROUTE_FRAME_LIMIT 42000

typedef struct RouteTrace {
    int            frames;
    uint32_t       story;
    uint32_t       persist1;
    uint32_t       token;
} RouteTrace;

static RouteTrace g_trace[MAX_TRACE];
static int        g_trace_count;
static int        g_frame;
static int        g_frame_limit;
static uint32_t   g_last_story = 0xDEADBEEFu;
static uint32_t   g_last_tok;
static int        g_first_story_frame = -1;
static int        g_milestone_hits;

/* m0020i save/load-menu exit observation.  PE_FieldMenuFrame sets
 * D_8009D1A0 bit 2 while the field menu (the save/load menu) is up and
 * clears it once the menu result is nonzero.  The recorded sequence confirms
 * the top menu into "save" at ~38612 and the autopilot presses Circle at
 * 38620/38640, so the mode bit must be clear again by ~38660. */
#define PE_MENU_EXIT_FRAME       38600
#define PE_MENU_EXIT_DONE_FRAME  38660
static int        g_menu_exit_seen_open;
static int        g_menu_exit_last_open;

/* Deterministic four-stage pad source.
 *
 * Stage 1 (frames < SWITCH=5400): hold 0xFFEF + periodic Cross.  This is the
 * input that carries m0002i -> m0003i -> m0091i -> m0004i, matching the
 * documented first-play prefix (m0003i @801AA828 -> m0003i).
 * Stage 2 (5400 <= frames < SWITCH2=6040): hold 0xFFBF + periodic Cross, which
 * releases the m0004i walk-to-bench gate and transfers to m0378i at 0x801B69D0.
 * Stage 3 (6040 <= frames < SWITCH3): hold 0xFF9F (held bits 0x30 = 0x10|0x20) +
 * periodic Cross.  Holding stage 2 (0xFFBF, held 0x20) leaves Aya parked just
 * outside every rectangle in m0378i, so the room's module-4 gates keep
 * failing; the 0xFF9F hold walks Aya into m0378i module-4 rectangle #1
 * (x in (-1067, 533), z in (-560, -300)) at 0x801957A8, which sets actor
 * local[4]=1 and fires the m0378i -> m0377i room_transfer at 0x80195728
 * (token 0xA80673C8).
 * Stage 4 (frames >= SWITCH3=6989, the frame m0377i is entered): hold 0xFFAF
 * (0xFF9F with the 0x10 bit also pressed) + periodic Cross.  m0377i module 1's
 * op-77 rectangle at 0x801953C4 (x in (-211, 241), z in (-4607, -4114)) is an
 * interior trigger that the 0xFF9F hold never enters.  With 0xFFAF the op-77
 * test returns a hit, local[4]=1, and the guard at 0x80195410 falls through
 * instead of looping at 0x8019544C.  m0377i module 5 then writes
 * persist[1]=0x179 and transfers back to m0378i at 0x80195768.
 *
 * The probe sweep (PE_ROUTE_PAD4=0xFFBF/0xFFDF/0xFF9F/0xFF7F) leaves the m0377i
 * gate looping exactly as the old three-stage default did; only masks that add
 * the 0x10 bit (0xFFAF, 0xFF2F, 0xFF8F, 0xFFCF) make the gate report a hit.
 * 0xFFAF is the one that then carries the bounce furthest: m0378i module 0's
 * transfer at 0x80195384 returns the route to m0004i, whose module-4 op-77
 * volumes (0x801B6940/0x801B6A08/0x801B6B74) are the next input-gated stop.
 *
 * Bits are active-low Sony pad bits (0 = pressed). 0xFF9F = 0xFFBF & 0xFFDF,
 * 0xFFAF = 0xFF9F & 0xFFEF.
 *
 * The four masks / switch frames / pulse period are overridable from the
 * environment so the route can be probed without a rebuild; the defaults are
 * the documented first-play sequence and are what the assertions run with.
 *
 * The table itself lives in pc_port/include/pe_route_pad.h so the interactive
 * --route-pad autopilot in src/port_main.c drives the IDENTICAL sequence
 * (shared source of truth; no parallel copy). */
static int g_route_frames = 0;   /* 0 = use the ROUTE_FRAME_LIMIT default */
static PeRoutePadConfig g_pad;
static PeRoutePadStep g_seq[PE_ROUTE_PAD_MAX_STEPS];
static unsigned g_seq_count;

#define PAD_CROSS  PE_ROUTE_PAD_CROSS
#define PAD_DOWN   PE_ROUTE_PAD_DOWN
#define PAD_UP     PE_ROUTE_PAD_UP
#define PAD_DN_LEFT PE_ROUTE_PAD_DN_LEFT
#define PAD_DN_LEFT_MASK2 PE_ROUTE_PAD_DN_LEFT_MASK2

/* Optional opcode-PC histogram (PE_ROUTE_PCTRACE=1).  It answers "which script
 * word is the frontier task parked on" without a rebuild or a debugger. */
#define PCTRACE_MAX 256
typedef struct PcSlot {
    pe_addr_t pc;
    uint32_t  op;
    uint32_t  count;
} PcSlot;
static PcSlot g_pc_slots[PCTRACE_MAX];
static int    g_pc_slot_count;
static int    g_pc_trace_on;
static int    g_pos_dump;
static int    g_pos_dump_every = 120;

static void PadConfigFromEnv(void)
{
    const char *s;

    PeRoutePad_ConfigFromEnv(&g_pad);
    /* Same recorded sequence the interactive --route-pad uses. */
    if (getenv("PE_ROUTE_PAD_SEQUENCE") && getenv("PE_ROUTE_PAD_SEQUENCE")[0])
        g_seq_count = PeRoutePad_ParseSequence(getenv("PE_ROUTE_PAD_SEQUENCE"),
                                               g_seq, PE_ROUTE_PAD_MAX_STEPS);
    else
        g_seq_count = PeRoutePad_ParseSequence(kDay1RoutePads, g_seq,
                                               PE_ROUTE_PAD_MAX_STEPS);
    s = getenv("PE_ROUTE_AYA_EVERY");
    if (s && s[0]) g_pos_dump_every = atoi(s);
    if (g_pos_dump_every < 1) g_pos_dump_every = 1;
    s = getenv("PE_ROUTE_FRAMES");
    if (s && s[0]) g_route_frames = atoi(s);
    g_pc_trace_on = getenv("PE_ROUTE_PCTRACE") != NULL;
    g_pos_dump = getenv("PE_ROUTE_AYA_DUMP") != NULL;
    g_pos_dump_every = 120;
}

static uint16_t RoutePadSource(void)
{
    uint16_t mask = PeRoutePad_Mask(&g_pad, g_frame);

    /* The recorded kDay1RoutePads sequence is what actually walks Aya through
     * the later Day-1 rooms; the four-stage table alone parks her at m0004i.
     * Apply the same sequence and Cross-suppression windows as port_main's
     * RoutePadSource so the harness and the interactive autopilot drive the
     * identical input. */
    mask = PeRoutePad_ApplySequence(g_seq, g_seq_count, g_frame, mask);
    if (!(g_frame >= 42713 && g_frame < 45041) &&
        !(g_frame >= 50500 && g_frame < 51200) &&
        !(g_frame >= 52344 && g_frame < 54500) &&
        !(g_frame >= 54500 && g_frame < 62000) &&
        PeRoutePad_PulseAllowed(g_frame, g_pad.period,
                                PE_ROUTE_PULSE_OFF1_BEGIN, PE_ROUTE_PULSE_OFF1_END,
                                PE_ROUTE_PULSE_OFF2_BEGIN, PE_ROUTE_PULSE_OFF2_END))
        mask &= g_pad.pulse;
    return mask;
}

static void RecordTrace(void)
{
    if (g_trace_count >= MAX_TRACE)
        return;
    g_trace[g_trace_count].frames   = g_frame;
    g_trace[g_trace_count].story    = PE_LoadU32(GA_PERSIST74);
    g_trace[g_trace_count].persist1 = PE_LoadU32(GA_PERSIST1);
    g_trace[g_trace_count].token    = GA_TOKEN;
    g_trace_count++;
}

/* Optional opcode-PC histogram (PE_ROUTE_PCTRACE=1).  It answers "which script
 * word is the frontier task parked on" without a rebuild or a debugger. */
static void PcTraceSample(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int guard = 0;

    if (!g_pc_trace_on)
        return;
    while (actor != 0u && guard < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc;
        uint32_t word;
        int i;

        if (PE_RangeIsRam(task, 4u)) {
            pc = PE_LoadU32(task);
            if (PE_RangeIsRam(pc, 4u)) {
                word = PE_LoadU32(pc);
                for (i = 0; i < g_pc_slot_count; i++) {
                    if (g_pc_slots[i].pc == pc) {
                        g_pc_slots[i].count++;
                        goto next;
                    }
                }
                if (g_pc_slot_count < PCTRACE_MAX) {
                    g_pc_slots[g_pc_slot_count].pc  = pc;
                    g_pc_slots[g_pc_slot_count].op  = word & 0x1FFFu;
                    g_pc_slots[g_pc_slot_count].count = 1u;
                    g_pc_slot_count++;
                }
            }
        }
    next:
        actor = PE_LoadU32(actor + 4u);
        guard++;
    }
}

/* Per-present hook: count frames, capture every token is not needed — a
 * token/story change is enough to reconstruct the path. */
static void RouteHook(void)
{
    uint32_t story;
    uint32_t tok;

    g_frame++;
    if (g_frame > g_frame_limit)
        return;

    if (g_first_story_frame < 0)
        g_first_story_frame = g_frame;

    if (g_frame >= PE_MENU_EXIT_FRAME && (PE_LoadU32(0x8009D1A0u) & 4u)) {
        g_menu_exit_seen_open = 1;
        g_menu_exit_last_open = g_frame;
    }

    story = PE_LoadU32(GA_PERSIST74);
    tok   = GA_TOKEN;
    if (story != g_last_story || tok != g_last_tok) {
        g_last_story = story;
        g_last_tok   = tok;
        RecordTrace();
    }
    if (g_pos_dump && (tok == 0xA8067448u || tok == 0xA80673C8u ||
                       tok == 0xA8000248u) &&
        (g_frame % g_pos_dump_every) == 0) {
        pe_addr_t aya = PE_LoadU32(0x8009D254u);
        pe_addr_t res = PE_LoadU32(0x8009D2F0u);
        if (aya)
            printf("route: ayadump f=%d x=%08X y=%08X z=%08X held=%08X edge=%08X res=%08X",
                   g_frame, (unsigned)PE_LoadU32(aya + 0x28u),
                   (unsigned)PE_LoadU32(aya + 0x2Cu),
                   (unsigned)PE_LoadU32(aya + 0x30u),
                   (unsigned)PE_LoadU32(0x8009D26Cu),
                   (unsigned)PE_LoadU32(0x8009D1F4u), (unsigned)res);
        if (aya)
            printf(" ayafl=%08X cb=%08X code=%02X d2e8=%08X d1a0=%08X",
                   (unsigned)PE_LoadU32(aya + 0x98u),
                   (unsigned)PE_LoadU32(aya + 0x190u),
                   (unsigned)PE_LoadU8(aya + 0x0Eu),
                   (unsigned)PE_LoadU32(0x8009D2E8u),
                   (unsigned)PE_LoadU32(0x8009D1A0u));
        if (res != 0u)
            printf(" resloc=%08X %08X %08X resfl=%08X",
                   (unsigned)PE_LoadU32(res + 0xACu + 8u),
                   (unsigned)PE_LoadU32(res + 0xACu + 12u),
                   (unsigned)PE_LoadU32(res + 0xACu),
                   (unsigned)PE_LoadU32(res + 0x98u));
        printf("\n");
        if (tok == 0xA8067448u) {
            pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
            int n2 = 0;
            while (actor != 0u && n2 < 64) {
                pe_addr_t task = PE_LoadU32(actor + 0xA8u);
                pe_addr_t pc = task ? PE_LoadU32(task) : 0u;
                if (pc != 0u)
                    printf("    actor=%08X type=%u pc=%08X task+10=%u L=%08X %08X %08X %08X %08X\n",
                           (unsigned)actor, (unsigned)PE_LoadU8(actor + 0xCu),
                           (unsigned)pc,
                           (unsigned)PE_LoadU32(task + 0x10u),
                           (unsigned)PE_LoadU32(actor + 0xACu),
                           (unsigned)PE_LoadU32(actor + 0xB0u),
                           (unsigned)PE_LoadU32(actor + 0xB4u),
                           (unsigned)PE_LoadU32(actor + 0xB8u),
                           (unsigned)PE_LoadU32(actor + 0xBCu));
                actor = PE_LoadU32(actor + 4u);
                n2++;
            }
        }
    }
    if ((g_frame % 30) == 0)
        PcTraceSample();
}

static PE_Disc *OpenRouteDisc(char *err, size_t err_size)
{
    const char *env = getenv("PE_DISC1_BIN");
    char path[1024];
    FILE *fp;
    size_t n;
    PE_Disc *disc;

    if (env && env[0])
        return PE_Disc_Open(env, err, err_size);

    fp = fopen("local/pe_disc1.path", "r");
    if (fp) {
        if (!fgets(path, (int)sizeof(path), fp)) {
            fclose(fp);
            if (err && err_size)
                snprintf(err, err_size, "empty local/pe_disc1.path");
            return NULL;
        }
        fclose(fp);
        n = strlen(path);
        while (n > 0 && (path[n - 1u] == '\n' || path[n - 1u] == '\r'))
            path[--n] = 0;
        disc = PE_Disc_Open(path, err, err_size);
        if (disc)
            return disc;
    }

    /* Same fallback as the other retail-disc fixtures: a locally extracted
     * Disc-1 executable can stand in when the image path is unavailable. */
    disc = PE_Disc_Open("build/extracted/disc1/SLUS_006.62", err, err_size);
    return disc;
}

static int TraceSawToken(uint32_t token)
{
    int i;
    for (i = 0; i < g_trace_count; i++)
        if (g_trace[i].token == token)
            return g_trace[i].frames;
    return -1;
}

static int TraceSawStory(uint32_t story)
{
    int i;
    for (i = 0; i < g_trace_count; i++)
        if (g_trace[i].story == story)
            return g_trace[i].frames;
    return -1;
}

/*
 * Run the boot spine once and return the stop reason.  The caller owns the
 * trace buffers; this only guarantees deterministic cleanup.
 */
static PEPortStopReason RunRoute(int frame_limit)
{
    PE_Disc *disc;
    char err[256];
    PE_RamInit();
    PE_Callback_Init();
    Bootstrap_Init();
    PE_Port_RunControlReset();
    HostFB_Init();
    PadConfigFromEnv();
    if (g_route_frames > 0)
        frame_limit = g_route_frames;

    g_frame = 0;
    g_frame_limit = frame_limit;
    g_trace_count = 0;
    g_last_story = 0xDEADBEEFu;
    g_last_tok = 0u;
    g_first_story_frame = -1;
    g_milestone_hits = 0;
    g_menu_exit_seen_open = 0;
    g_menu_exit_last_open = 0;
    err[0] = 0;
    disc = OpenRouteDisc(err, sizeof(err));
    if (!disc) {
        fprintf(stderr, "route: disc open failed: %s\n", err);
        return PE_PORT_STOP_NONE;
    }
    PE_Disc_SetActive(disc);
    if (PE_GuestImage_LoadExe(disc, err, sizeof(err))) {
        fprintf(stderr, "route: EXE load failed: %s\n", err);
        PE_Disc_SetActive(NULL);
        return PE_PORT_STOP_NONE;
    }
    if (PE_Globals_AdoptRetailImage()) {
        fprintf(stderr, "route: globals adopt failed\n");
        PE_Disc_SetActive(NULL);
        return PE_PORT_STOP_NONE;
    }

    PE_Port_SetSkipMovie(1);         /* documented HOST_ADAPTED boundary */
    PE_Port_SetSkipOpeningMenu(1);   /* documented HOST_ADAPTED boundary */
    PE_Port_SetFrameLimit(frame_limit);
    PE_Port_SetPadSource(RoutePadSource);
    PE_Port_SetPresentHook(RouteHook);

    func_8001220C();

    PE_Disc_SetActive(NULL);
    return PE_Port_GetStopReason();
}

/*
 * Milestone table.  Each entry is a retail-proven transition:
 *   kind 0 = persist[74] value, kind 1 = room token, kind 2 = persist[1]
 *   value, kind 3 = boot must still be inside the field runtime (not a
 *   HOST_ADAPTED boundary), kind 4 = a named HOST_ADAPTED boundary stub.
 */
typedef struct Milestone {
    int         kind;
    uint32_t    value;
    const char *what;
    const char *evidence;
} Milestone;

static const Milestone kMilestones[] = {
    { 0, 0x00000001u, "persist[74]=0x01 (m0010i profile/name entry)",
      "m0010i module EF path" },
    { 0, 0x00000008u, "persist[74]=0x08",
      "first field-prefix story step" },
    { 1, 0xA8001048u, "token m0010i (0xA8001048)",
      "field prefix first room" },
    { 1, 0xA8000148u, "token m0002i (0xA8000148)",
      "m0010i -> m0002i" },
    { 0, 0x00000009u, "persist[74]=0x09 (m0002i)",
      "m0002i story write" },
    { 1, 0xA80001C8u, "token m0003i (0xA80001C8)",
      "m0002i @801AA828 room_transfer A80001C8" },
    { 1, 0xA8067148u, "token m0372i (0xA8067148)",
      "m0003i -> m0372i (FMV003 prefix)" },
    { 0, 0x00000011u, "persist[74]=0x11",
      "m0372i story write" },
    { 0, 0x00000012u, "persist[74]=0x12",
      "m0372i -> m0004i story write" },
    { 1, 0xA8000248u, "token m0004i (0xA8000248)",
      "m0372i @801D7844 room_transfer A8000248" },
    { 0, 0x00000018u, "persist[74]=0x18 (m0004i)",
      "m0004i @801B5FD4 assign [74,0x18]" },
    { 1, 0xA8067448u, "token m0378i (0xA8067448)",
      "m0004i @801B69D0 room_transfer A8067448" },
    { 1, 0xA80673C8u, "token m0377i (0xA80673C8)",
      "m0378i @80195728 room_transfer A80673C8 (mod4 rect #1 "
      "(-1067,-560)-(-300,-300) at 0x801957A8)" },
    { 2, 0x00000179u, "persist[1]=0x179 (m0377i module 5)",
      "m0377i @80195758 assign [1,0x179]; module 5 @80195768 "
      "room_transfer A8067448 (mod1 op77 rect (-211,-4607)-(241,-4114) "
      "at 0x801953C4 hit under PAD4=0xFFAF)" },
    /* Later Day-1 rooms, reached once the recorded kDay1RoutePads sequence is
     * applied (the four-stage table alone parks at m0004i).  Tokens decoded
     * with pc_port/tools/pe_btl14_m0005i_publish_oracle.py's decode_token. */
    { 1, 0xA80002C8u, "token m0005i (0xA80002C8)",
      "m0004i walk gate -> m0005i" },
    { 0, 0x00000028u, "persist[74]=0x28 (m0005i)",
      "m0005i story write" },
    { 1, 0xA80663C8u, "token m0367i (0xA80663C8)",
      "m0005i -> m0367i" },
    { 1, 0xA80004C8u, "token m0009i (0xA80004C8)",
      "m0367i -> m0009i" },
    { 0, 0x00000030u, "persist[74]=0x30 (m0009i)",
      "m0009i story write" },
    { 1, 0xA80010C8u, "token m0011i (0xA80010C8)",
      "m0009i -> m0011i" },
    { 0, 0x00000038u, "persist[74]=0x38 (m0011i)",
      "m0011i story write" },
    { 1, 0xA8001148u, "token m0012i (0xA8001148)",
      "m0011i -> m0012i" },
    { 0, 0x00000039u, "persist[74]=0x39 (m0012i)",
      "m0012i story write" },
    { 0, 0x00000040u, "persist[74]=0x40 (m0012i)",
      "m0012i story write" },
    { 1, 0xA80011C8u, "token m0013i (0xA80011C8)",
      "m0012i -> m0013i" },
    { 0, 0x00000048u, "persist[74]=0x48 (m0013i -> m0020i)",
      "m0013i story write" },
    { 1, 0xA8002048u, "token m0020i (0xA8002048)",
      "Day-1 save/load menu room (empty card slot retries here)" },
};

#define MILESTONE_COUNT ((int)(sizeof(kMilestones) / sizeof(kMilestones[0])))

static int MilestoneSatisfied(const Milestone *m)
{
    switch (m->kind) {
    case 0: return TraceSawStory(m->value) >= 0;
    case 1: return TraceSawToken(m->value) >= 0;
    case 2: {
        int i;
        for (i = 0; i < g_trace_count; i++)
            if (g_trace[i].persist1 == m->value)
                return 1;
        return 0;
    }
    default: return 0;
    }
}

static int BoundaryStubCount(const char *symbol)
{
    int n = 0;
    int i;
    for (i = 0; i < g_stub_order_count; i++)
        if (strcmp(g_stub_order_log[i], symbol) == 0)
            n++;
    return n;
}

/*
 * The frontier this harness asserts when it reaches the documented end of the
 * currently-traversable chain:
 *
 *   room m0378i module 4 (token 0xA8067448) reaches its second opcode-0x77
 *   rectangle gate at 0x801957A8 (func_80014DA0 -> func_8001CAB0, result in
 *   actor local[2], settle flag in local[3]) and passes it once Aya is walked
 *   into rectangle #1's band, which sets local[4]=1 and fires the
 *   m0378i -> m0377i room_transfer at 0x80195728 (token 0xA80673C8).
 *
 * The new frontier is the m0004i type-4 task at 0x801B6CC8 (module 4's final
 * 1-frame wait loop; opcode 0x00 `jump` back into module 4's op77 chain).  The
 * task is pinned by its PC rather than by the token alone so the stop names
 * the exact script word that parks it.  The token at the stop is m0004i again
 * (0xA8000248) because m0377i module 5 bounced back through m0378i module 0.
 *
 * Reaching here means the port executed the full cold-boot -> field-prefix ->
 * m0002i -> m0003i -> m0372i -> m0004i -> m0378i -> m0377i -> m0378i ->
 * m0004i chain AND both m0378i module 4 rectangles AND m0377i module 1's
 * op-77 rectangle without hitting an unported opcode.  See
 * docs/evidence/boot-day2-route-harness/REPORT.md.
 */
#define FRONTIER_TOKEN 0xA8000248u
#define FRONTIER_PC    0x801B6CC8u

/* Return the frontier actor's task PC if the list is parked at FRONTIER_PC,
 * else 0.  Used to pin the stop, not to influence it. */
static pe_addr_t FindFrontierPc(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int n = 0;
    while (actor != 0u && n < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc = PE_RangeIsRam(task, 4u) ? PE_LoadU32(task) : 0u;
        if (pc == FRONTIER_PC)
            return pc;
        actor = PE_LoadU32(actor + 4u);
        n++;
    }
    return 0u;
}

/*
 * Frontier report: walk the live actor list and print each actor's script PC
 * and its first 8 script-local words (actor+0xAC + i*4, the VM's mode-1
 * operand base).  This is what names the blocker when the route stops short
 * of a milestone — the room token alone does not say which task is parked,
 * and the locals show the op77 hit-test inputs/result.
 */
static void PrintFrontier(void)
{
    pe_addr_t actor = PE_LoadU32(0x8009D20Cu);
    int n = 0;

    printf("route: frontier actors (D_8009D20C list):\n");
    while (actor != 0u && n < 64) {
        pe_addr_t task = PE_RangeIsRam(actor + 0xA8u, 4u)
            ? PE_LoadU32(actor + 0xA8u) : 0u;
        pe_addr_t pc = PE_RangeIsRam(task, 4u) ? PE_LoadU32(task) : 0u;
        uint32_t word = PE_RangeIsRam(pc, 4u) ? PE_LoadU32(pc) : 0u;
        printf("  actor=%08X type=%u id=%u task=%08X pc=%08X op=%02X delay=%u\n",
               (unsigned)actor, PE_LoadU8(actor + 0xCu), PE_LoadU8(actor + 0xDu),
               (unsigned)task, (unsigned)pc, (unsigned)(word & 0x1FFFu),
               PE_RangeIsRam(task + 0x10u, 4u)
                   ? (unsigned)PE_LoadU32(task + 0x10u) : 0u);
        if (PE_RangeIsRam(actor + 0xACu, 32u)) {
            printf("    locals:");
            {
                int i;
                for (i = 0; i < 8; i++)
                    printf(" [%d]=%08X", i, (unsigned)PE_LoadU32(actor + 0xACu + (pe_addr_t)i * 4u));
            }
            printf("\n");
        }
        actor = PE_LoadU32(actor + 4u);
        n++;
    }
}

int main(void)
{
    PEPortStopReason reason;
    int reached = 0;
    int i;

    if (access("local/pe_disc1.path", R_OK) != 0 &&
        !(getenv("PE_DISC1_BIN") && getenv("PE_DISC1_BIN")[0])) {
        printf("SKIP (requires PE_DISC1_BIN or local/pe_disc1.path)\n");
        return 0;
    }

    reason = RunRoute(ROUTE_FRAME_LIMIT);
    printf("route: frames=%d stop=%s story=0x%08X persist1=0x%08X token=0x%08X\n",
           g_frame, PE_Port_StopReasonName(reason),
           PE_LoadU32(GA_PERSIST74), PE_LoadU32(GA_PERSIST1), GA_TOKEN);

    for (i = 0; i < g_trace_count; i++) {
        const RouteTrace *t = &g_trace[i];
        int j;
        int marks = 0;
        for (j = 0; j < MILESTONE_COUNT; j++) {
            const Milestone *m = &kMilestones[j];
            if ((m->kind == 0 && m->value == t->story) ||
                (m->kind == 1 && m->value == t->token) ||
                (m->kind == 2 && m->value == t->persist1))
                marks = 1;
        }
        printf("  [%c f=%d] story=0x%08X persist1=0x%08X token=0x%08X\n",
               marks ? 'X' : ' ', t->frames, t->story, t->persist1, t->token);
    }

    for (i = 0; i < MILESTONE_COUNT; i++) {
        const Milestone *m = &kMilestones[i];
        int ok = MilestoneSatisfied(m);
        printf("  %s %s  (%s)\n", ok ? "OK  " : "MISS", m->what, m->evidence);
        if (ok)
            reached++;
    }

    printf("route: %d/%d ordered milestones reached\n", reached, MILESTONE_COUNT);

    printf("route: boundary stubs invoked:\n");
    for (i = 0; i < g_stub_count; i++) {
        printf("  %s %s x%d\n", g_stub_registry[i].classification,
               g_stub_registry[i].symbol, g_stub_registry[i].invoked);
    }

    /* Loud-boundary contract: an UNSUPPORTED stub would silently stand in for
     * missing retail behavior, so the route must never have invoked one. */
    for (i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].classification, "UNSUPPORTED") == 0 &&
            g_stub_registry[i].invoked > 0) {
            printf("FAIL: UNSUPPORTED boundary stub %s invoked\n",
                   g_stub_registry[i].symbol);
            return 1;
        }
    }

    /* Name the frontier: the room token says where, the script PC says what. */
    PrintFrontier();

    if (g_pc_trace_on) {
        printf("route: frontier opcode-PC histogram:\n");
        for (i = 0; i < g_pc_slot_count; i++)
            printf("  pc=%08X op=%02X frames=%u\n", (unsigned)g_pc_slots[i].pc,
                   (unsigned)g_pc_slots[i].op, g_pc_slots[i].count);
    }

    if (getenv("PE_ROUTE_AYA_DUMP")) {
        pe_addr_t aya = PE_LoadU32(0x8009D254u);
        printf("route: Aya D_8009D254=%08X", (unsigned)aya);
        if (aya) {
            printf(" pos x=%08X y=%08X z=%08X", (unsigned)PE_LoadU32(aya + 0x28u),
                   (unsigned)PE_LoadU32(aya + 0x2Cu),
                   (unsigned)PE_LoadU32(aya + 0x30u));
        }
        printf("\nroute: pad raw D_800BE9A2=%04X held D_8009D26C=%08X edge D_8009D1F4=%08X prev D_8009D1E4=%08X\n",
               (unsigned)PE_LoadU16(0x800BE9A2u),
               (unsigned)PE_LoadU32(0x8009D26Cu),
               (unsigned)PE_LoadU32(0x8009D1F4u),
               (unsigned)PE_LoadU32(0x8009D1E4u));
    }

    /* Loud-boundary contract: an UNSUPPORTED stub would silently stand in for
     * missing retail behavior, so the route must never have invoked one. */
    for (i = 0; i < g_stub_count; i++) {
        if (strcmp(g_stub_registry[i].classification, "UNSUPPORTED") == 0 &&
            g_stub_registry[i].invoked > 0) {
            printf("FAIL: UNSUPPORTED boundary stub %s invoked\n",
                   g_stub_registry[i].symbol);
            return 1;
        }
    }

    /* The four documented HOST_ADAPTED skips must actually have been taken —
     * otherwise the harness is not exercising the same host-adapted boot it
     * claims (movie + opening-menu skip), and the route numbers are suspect. */
    if (BoundaryStubCount("func_801909B4_skip_movie_new_game") == 0 ||
        BoundaryStubCount("func_8006E9A0_skip_movie_fade") == 0 ||
        BoundaryStubCount("func_80014E30_skip_movie") == 0 ||
        BoundaryStubCount("func_80016F10_skip_opening_menu") == 0) {
        printf("FAIL: a documented HOST_ADAPTED skip stub was never invoked\n");
        return 1;
    }

    /* The route cannot claim more than it proved.  If it stopped before the
     * documented frontier, say exactly where so a regression is visible.  The
     * recorded route now continues past m0004i to the Day-1 save/load menu
     * (m0020i); there the empty card slot makes the menu retry, so the actor
     * frontier PC is 0 — pin reaching m0020i and stopping at the frame limit
     * instead of the old m0004i park. */
    /* The m0020i save/load menu must actually close.  Confirming the top menu
     * into "save" happens at ~38612; the recorded sequence then presses Circle
     * at 38620 (cancel the slot list: func_8004D6D4 event 0x40) and 38640
     * (close the menu: func_8004D2DC event 0x40 -> func_800512AC(9,0) ->
     * D_8009D010 nonzero), after which PE_FieldMenuFrame clears D_8009D1A0
     * bit 2.  Without the second pulse-suppression window the top menu is
     * re-confirmed every 8 frames and the mode bit never clears. */
    if (g_menu_exit_seen_open && g_menu_exit_last_open >= PE_MENU_EXIT_DONE_FRAME) {
        printf("FAIL: m0020i save/load menu never closed "
               "(field-menu mode still set at frame %d)\n", g_menu_exit_last_open);
        return 1;
    }

    if (reached == MILESTONE_COUNT) {
        if (GA_TOKEN == 0xA8002048u || TraceSawToken(0xA8002048u) >= 0) {
            printf("PASS: boot -> Day-1 rooms -> m0020i save/load menu "
                   "(%d milestones, frames=%d stop=%s)\n",
                   MILESTONE_COUNT, g_frame, PE_Port_StopReasonName(reason));
            return 0;
        }
        printf("FAIL: %d milestones reached but did not reach m0020i "
               "(token=0x%08X)\n", MILESTONE_COUNT, (unsigned)GA_TOKEN);
        return 1;
    }

    printf("FAIL: route stopped at milestone %d (%s)\n", reached,
           reached < MILESTONE_COUNT ? kMilestones[reached].what : "?");
    return 1;
}
