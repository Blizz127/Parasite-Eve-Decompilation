/*
 * pe_route_pad.h — canonical deterministic route pad for the boot -> Day-2
 * route proof.
 *
 * Single source of truth shared by the two consumers of the scripted input:
 *
 *   * pc_port/tests/test_route_boot_day2.c — the milestone harness that
 *     asserts the address-exact persist[]/token chain, and
 *   * pc_port/src/port_main.c — the interactive `--route-pad` autopilot.
 *
 * Keeping the stage table here is what makes the windowed run and the harness
 * walk the same route: both feed PeRoutePad_Mask() the identical masks at the
 * identical frame thresholds.
 *
 * STAGE TABLE (bits are active-low Sony pad bits, 0 = pressed):
 *
 *   Stage 1 (frame < 5400): hold 0xFFEF (DOWN) + periodic Cross. Carries
 *     m0002i -> m0003i -> m0091i -> m0004i, the documented first-play prefix.
 *   Stage 2 (5400 <= frame < 6040): hold 0xFFBF (UP) + periodic Cross.
 *     Releases the m0004i walk-to-bench gate, transfers to m0378i at
 *     0x801B69D0.
 *   Stage 3 (6040 <= frame < 6989): hold 0xFF9F (DOWN|UP, held bits 0x30) +
 *     periodic Cross. Walks Aya into m0378i module-4 rectangle #1
 *     (x in (-1067, 533), z in (-560, -300)) at 0x801957A8, sets local[4]=1
 *     and fires the m0378i -> m0377i room_transfer at 0x80195728
 *     (token 0xA80673C8).
 *   Stage 4 (frame >= 6989, the frame m0377i is entered): hold 0xFFAF
 *     (0xFF9F with the 0x10 bit also pressed) + periodic Cross. m0377i module
 *     1's op-77 rectangle at 0x801953C4 (x in (-211, 241), z in
 *     (-4607, -4114)) is an interior trigger the 0xFF9F hold never enters.
 *     With 0xFFAF the op-77 test returns a hit, local[4]=1, and the guard at
 *     0x80195410 falls through instead of looping at 0x8019544C. m0377i
 *     module 5 then writes persist[1]=0x179 and transfers back to m0378i at
 *     0x80195768; m0378i module 0 bounces to m0004i, whose module-4 type-4
 *     task parks at 0x801B6CC8 (the current frontier).
 *
 * Probe sweep: PE_ROUTE_PAD4=0xFFBF/0xFFDF/0xFF9F/0xFF7F leaves the m0377i
 * gate looping exactly as the old three-stage default (0xFF9F hold) did; only
 * masks that add the 0x10 bit (0xFFAF, 0xFF2F, 0xFF8F, 0xFFCF) make the gate
 * report a hit. Setting PE_ROUTE_PAD4=0xFF9F reproduces the old three-stage
 * behaviour exactly (stage 4 continues to hold the stage-3 mask).
 *
 * 0xFF9F = 0xFFBF & 0xFFDF, 0xFFAF = 0xFF9F & 0xFFEF.
 *
 * The masks / switch frames / pulse period are overridable from the
 * environment so the route can be probed without a rebuild; the defaults are
 * the documented first-play sequence and are what the assertions run with.
 */
#ifndef PC_PORT_PE_ROUTE_PAD_H
#define PC_PORT_PE_ROUTE_PAD_H

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#define PE_ROUTE_PAD_CROSS         0xBFFFu
#define PE_ROUTE_PAD_DOWN          0xFFEFu
#define PE_ROUTE_PAD_UP            0xFFBFu
#define PE_ROUTE_PAD_DN_LEFT       0xFF9Fu
#define PE_ROUTE_PAD_DN_LEFT_MASK2 0xFFAFu
#define PE_ROUTE_PAD_CROSS_PERIOD  8

/*
 * Automatic-Cross-pulse suppression windows.
 *
 * The autopilot pulse is "mash confirm": every `period` frames it presses
 * Cross.  It has to be switched off wherever mashing would undo what the
 * route is doing, so two windows are recorded:
 *
 *   [33620, 35300)  the m0004i segment, which the recorded sequence covers
 *                   with explicit held buttons.
 *   [38620, ...)    the Day-1 save point.  The automatic Cross at 38611
 *                   opens the save menu; the recorded sequence then presses
 *                   Circle twice -- 38620 is consumed by the slot-list window
 *                   (func_8004D6D4 event 0x40) and 38640 reaches the file
 *                   menu (func_8004D2DC event 0x40 -> func_8005C1EC(0) ->
 *                   func_800512AC(9,0) -> D_8009D010 nonzero ->
 *                   PE_FieldMenuFrame clears the field-menu mode).  With the
 *                   pulse still on the top menu is re-confirmed every 8
 *                   frames and the menu never closes.
 *
 * BOTH consumers -- the interactive --route-pad port and the boot -> Day-2
 * harness -- must apply the identical rule, so they share
 * PeRoutePad_PulseAllowed(); the harness previously carried only the first
 * window and therefore stopped reproducing the port at the save point.
 *
 * Bounds are inclusive-begin / exclusive-end.  The second window is open
 * ended on purpose: ending it anywhere in the recorded tail re-enables
 * invented Cross mashing, which re-enters the file menu.  With the window
 * closed at 42000 the port stops at the "card operation unresolved call"
 * boundary at ~f=44300; left open, it runs to ~f=61500 before stopping at the
 * unported file-menu page func_8004AD9C.  Same room either way, so the open
 * window is both more faithful and the longer route.
 */
#define PE_ROUTE_PULSE_OFF1_BEGIN 33620
#define PE_ROUTE_PULSE_OFF1_END   35300
#define PE_ROUTE_PULSE_OFF2_BEGIN 38620
#define PE_ROUTE_PULSE_OFF2_END   0x7FFFFFFF

/* True when the automatic Cross pulse may be applied on `frame`.  Callers
 * only differ in how they source the window bounds (compiled defaults vs.
 * PE_ROUTE_PULSE* overrides). */
static inline int PeRoutePad_PulseAllowed(int frame, int period,
                                          int off1_begin, int off1_end,
                                          int off2_begin, int off2_end)
{
    if (period < 1 || (frame % period) != 3)
        return 0;
    if (frame >= off1_begin && frame < off1_end)
        return 0;
    if (frame >= off2_begin && frame < off2_end)
        return 0;
    return 1;
}

#define PE_ROUTE_PAD_SWITCH_FRAME   5400
#define PE_ROUTE_PAD_SWITCH2_FRAME  6040
#define PE_ROUTE_PAD_SWITCH3_FRAME  6989

typedef struct PeRoutePadConfig {
    uint16_t stage1;
    uint16_t stage2;
    uint16_t stage3;
    uint16_t stage4;
    uint16_t pulse;
    int      switch_frame;
    int      switch2_frame;
    int      switch3_frame;
    int      period;
} PeRoutePadConfig;

static inline void PeRoutePad_Defaults(PeRoutePadConfig *c)
{
    c->stage1         = PE_ROUTE_PAD_DOWN;
    c->stage2         = PE_ROUTE_PAD_UP;
    c->stage3         = PE_ROUTE_PAD_DN_LEFT;
    c->stage4         = PE_ROUTE_PAD_DN_LEFT_MASK2;
    c->pulse          = PE_ROUTE_PAD_CROSS;
    c->switch_frame   = PE_ROUTE_PAD_SWITCH_FRAME;
    c->switch2_frame  = PE_ROUTE_PAD_SWITCH2_FRAME;
    c->switch3_frame  = PE_ROUTE_PAD_SWITCH3_FRAME;
    c->period         = PE_ROUTE_PAD_CROSS_PERIOD;
}

static inline uint16_t PeRoutePad_EnvHex16(const char *name, uint16_t dflt)
{
    const char *s = getenv(name);
    return (s && s[0]) ? (uint16_t)strtoul(s, NULL, 16) : dflt;
}

static inline void PeRoutePad_ConfigFromEnv(PeRoutePadConfig *c)
{
    const char *s;

    PeRoutePad_Defaults(c);
    c->stage1 = PeRoutePad_EnvHex16("PE_ROUTE_PAD1", c->stage1);
    c->stage2 = PeRoutePad_EnvHex16("PE_ROUTE_PAD2", c->stage2);
    c->stage3 = PeRoutePad_EnvHex16("PE_ROUTE_PAD3", c->stage3);
    c->stage4 = PeRoutePad_EnvHex16("PE_ROUTE_PAD4", c->stage4);
    c->pulse  = PeRoutePad_EnvHex16("PE_ROUTE_PULSE", c->pulse);
    s = getenv("PE_ROUTE_SWITCH");
    if (s && s[0]) c->switch_frame = atoi(s);
    s = getenv("PE_ROUTE_SWITCH2");
    if (s && s[0]) c->switch2_frame = atoi(s);
    s = getenv("PE_ROUTE_SWITCH3");
    if (s && s[0]) c->switch3_frame = atoi(s);
    s = getenv("PE_ROUTE_PERIOD");
    if (s && s[0]) c->period = atoi(s);
    if (c->period < 1) c->period = 1;
}

/*
 * Recorded pad sequence ("frame:mask,frame:mask,...").  This is the same input
 * the interactive --route-pad port parses from kDay1RoutePads: unlike the
 * four-stage PeRoutePad_Mask table, it carries the frame-exact held buttons
 * that walk Aya through the later Day-1 rooms.  Shared here so the route
 * harness and the port drive the identical sequence.
 */
#define PE_ROUTE_PAD_MAX_STEPS 4096

typedef struct PeRoutePadStep {
    int      frame;
    uint16_t mask;
} PeRoutePadStep;

/* Parse strictly-increasing frame:mask pairs.  Returns the number parsed;
 * stops at the first malformed or out-of-order entry (loud in the caller's
 * own validation if it cares). */
static inline unsigned PeRoutePad_ParseSequence(const char *s,
                                                PeRoutePadStep *steps,
                                                unsigned cap)
{
    unsigned n = 0;
    while (s && s[0]) {
        char *end;
        unsigned long frame, mask;
        errno = 0;
        frame = strtoul(s, &end, 10);
        if (errno || end == s || *end != ':' || frame > 0x7FFFFFFFul ||
            n == cap || (n && frame <= (unsigned long)steps[n - 1].frame))
            break;
        s = end + 1;
        errno = 0;
        mask = strtoul(s, &end, 16);
        if (errno || end == s || mask > 0xFFFFul || (*end && *end != ','))
            break;
        steps[n].frame = (int)frame;
        steps[n].mask = (uint16_t)mask;
        n++;
        s = *end ? end + 1 : end;
    }
    return n;
}

/* Apply the latest step at or before `frame`; earlier steps are cumulative
 * (a step holds until the next one). */
static inline uint16_t PeRoutePad_ApplySequence(const PeRoutePadStep *steps,
                                                unsigned count, int frame,
                                                uint16_t mask)
{
    for (unsigned i = 0; i < count && frame >= steps[i].frame; i++)
        mask = steps[i].mask;
    return mask;
}

/*
 * The pad mask for a given route frame.  `frame` is supplied by the caller
 * (the harness counts present-hook frames; the interactive port counts pad
 * reads) so the two keep their own clock while sharing this exact table.
 */
static inline uint16_t PeRoutePad_Mask(const PeRoutePadConfig *c, int frame)
{
    uint16_t m;

    if (c->switch3_frame > 0 && frame >= c->switch3_frame)
        m = c->stage4;
    else if (c->switch2_frame > 0 && frame >= c->switch2_frame)
        m = c->stage3;
    else if (frame >= c->switch_frame)
        m = c->stage2;
    else
        m = c->stage1;
    if ((frame % c->period) == 3)
        m &= c->pulse;
    return m;
}

#endif /* PC_PORT_PE_ROUTE_PAD_H */
