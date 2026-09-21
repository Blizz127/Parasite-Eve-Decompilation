/*
 * Phase 6E-B54K-R/Y/T — func_801909B4 through MoveImage, display setup, the
 * one-time overlay-local initializer, Disc-1 FMV (func_80192CE8), post-
 * movie environment restore, title present (func_8018F2F4), freelist seed,
 * title-object alloc (func_8018FBC0), title-loop header (bank flip +
 * 425DC with HOST_ADAPTED TestEvent), and the input-pump frontier at
 * 0x801911F8 (jal func_8003EB04).
 *
 * Retail overlay body: [0x801909B4,0x801918F8), 977 words.
 * HOST_ADAPTED --skip-movie still short-circuits to FinishTitleMenu(1).
 * Without it, Disc 1's saved bit plays FMV001; both arms then run title
 * present + freelist + one object alloc + loop header + one 425DC pump.
 * --skip-opening-menu returns New-Game after present/freelist, before
 * alloc / loop.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "stub_registry.h"

#include <string.h>

#define GA_DRAWENV_SOURCE_0  0x800BCDC8u
#define GA_DRAWENV_SOURCE_1  0x800BCE24u
#define GA_DISPENV_SOURCE_0  0x800BCE80u
#define GA_DISPENV_SOURCE_1  0x800BCE94u
#define GA_DRAWENV_COPY_0    0x801D1498u
#define GA_DRAWENV_COPY_1    0x801D14F4u
#define GA_DISPENV_COPY_0    0x801D1550u
#define GA_DISPENV_COPY_1    0x801D1564u

extern void func_8005E57C(int value);
extern void func_8005C1EC(int enabled);
extern void func_8005E6E4(int value);
extern void func_80042538(void);
extern void func_800425DC(void);
extern void func_8003EB04(void);
extern int func_80190660(void);
extern int func_80192CE8(int index);
extern void func_8018F2F4(void);
extern pe_addr_t func_8018FBC0(int type);
extern void func_80190064(void);
extern void func_8018F468(void);

static void TitleListSweepAndMerge(void)
{
    pe_addr_t node = PE_LoadU32(0x801D1370u);
    pe_addr_t prev = 0u;
    pe_addr_t pending;
    pe_addr_t tail;

    while (node != 0u) {
        pe_addr_t next = PE_LoadU32(node);
        if (PE_LoadU32(node + 0x30u) != 0u) {
            if (prev != 0u)
                PE_StoreU32(prev, next);
            else
                PE_StoreU32(0x801D1370u, next);
            if (PE_LoadU32(0x801D1374u) == node)
                PE_StoreU32(0x801D1374u, prev);
            PE_StoreU32(node, PE_LoadU32(0x801D136Cu));
            PE_StoreU32(0x801D136Cu, node);
            node = prev != 0u ? PE_LoadU32(prev) : PE_LoadU32(0x801D1370u);
            continue;
        }
        prev = node;
        node = next;
    }

    pending = PE_LoadU32(0x801D1378u);
    if (pending == 0u)
        return;
    tail = PE_LoadU32(0x801D1374u);
    if (tail != 0u)
        PE_StoreU32(tail, pending);
    else
        PE_StoreU32(0x801D1370u, pending);
    PE_StoreU32(0x801D1374u, PE_LoadU32(0x801D137Cu));
    PE_StoreU32(0x801D137Cu, 0u);
    PE_StoreU32(0x801D1378u, 0u);
}

static void TitleLoopPresent(void)
{
    pe_addr_t active;

    (void)func_80074DC0(0);
    active = PE_LoadU32(0x801D11C4u);
    if ((int16_t)PE_LoadU16(active + 0x74u) > 0) {
        RECT blit;
        int32_t w;
        int32_t h;

        blit.x = (int16_t)PE_LoadU16(active + 0x70u);
        blit.y = (int16_t)PE_LoadU16(active + 0x72u);
        blit.w = (int16_t)PE_LoadU16(active + 0x74u);
        blit.h = (int16_t)PE_LoadU16(active + 0x76u);
        w = (int32_t)blit.x;
        blit.x = (int16_t)((w + w + w) >> 1);
        if (PE_LoadU32(0x801D11C8u) == 0u)
            blit.y = (int16_t)(blit.y + 240);
        h = (int32_t)blit.w;
        blit.w = (int16_t)((h + h + h) >> 1);
        (void)func_8007506C(&blit, active + 0x8080u);
    }
    (void)func_80073A44(2);
    (void)func_80074A44(1);
    active = PE_LoadU32(0x801D11C4u);
    (void)func_80075424(active);
    (void)func_800755F0(active + 0x5Cu);
}

#define GA_TITLE_NODE_POOL   0x801D11CCu
#define GA_TITLE_NODE_END    0x801D1338u /* pool + 364 */
#define GA_TITLE_NODE_STRIDE 52u
#define GA_TITLE_FREELIST    0x801D136Cu
#define GA_TITLE_CALLBACK    0x8019319Cu
#define GA_TITLE_SAVED_BIT   0x801D1380u

static void CopyGuestBytes(pe_addr_t destination, pe_addr_t source,
                           uint32_t size)
{
    memcpy(PE_Translate(destination, size),
           PE_TranslateConst(source, size), size);
}

/* Original nonnegative-selector exit, 0x801916DC..0x801918C8.
 * The movie-skip entry still needs the title's ordinary teardown: its
 * standalone renderer and card timer must not survive into the field.
 * The current screen Y survives the saved-environment restoration. */
static int FinishTitleMenu(int selector)
{
    RECT clear_rect = {0, 0, 320, 480};
    uint16_t screen_y;

    func_80074D28(0);
    func_8005E6E4(0);
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    if (PE_Port_ShouldStop())
        return -1;
    screen_y = PE_LoadU16(GA_DISPENV_SOURCE_0 + 10u);

    CopyGuestBytes(GA_DRAWENV_SOURCE_0, GA_DRAWENV_COPY_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_SOURCE_1, GA_DRAWENV_COPY_1, 0x5Cu);
    CopyGuestBytes(GA_DISPENV_SOURCE_0, GA_DISPENV_COPY_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_SOURCE_1, GA_DISPENV_COPY_1, 0x14u);
    PE_StoreU16(GA_DISPENV_SOURCE_1 + 10u, screen_y);
    PE_StoreU16(GA_DISPENV_SOURCE_0 + 10u, screen_y);
    func_8005C1EC(0);
    if (PE_Port_ShouldStop())
        return -1;
    func_8005E57C(0);
    return selector;
}

/* 0x80190DB4..0x8019111C: copy live DRAWENV/DISPENV into the title
 * arenas, clear the HUD strips, then PutDrawEnv/PutDispEnv. */
static void PostMovieRestoreEnvironments(void)
{
    pe_addr_t arena0 = PE_LoadU32(0x801D11BCu);
    pe_addr_t arena1 = PE_LoadU32(0x801D11C0u);
    RECT clear_rect;
    uint32_t bank;
    pe_addr_t active;

    CopyGuestBytes(arena0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(arena1, GA_DRAWENV_SOURCE_1, 0x5Cu);
    CopyGuestBytes(arena0 + 0x5Cu, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(arena1 + 0x5Cu, GA_DISPENV_SOURCE_1, 0x14u);

    PE_StoreU8(arena0 + 0x18u, 0u);
    PE_StoreU8(arena1 + 0x18u, 0u);
    PE_StoreU16(arena0 + 0x7Cu, 0u);
    PE_StoreU16(arena1 + 0x7Cu, 0u);
    PE_StoreU16(arena0 + 0x74u, 0u);
    PE_StoreU16(arena1 + 0x74u, 0u);

    bank = PE_LoadU32(0x800ACDDCu);
    PE_StoreU32(0x801D11C8u, bank);
    PE_StoreU16(arena0 + 0x74u, 0u);
    PE_StoreU16(arena1 + 0x74u, 0u);

    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 480;
    clear_rect.h = 20;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 224;
    clear_rect.h = 16;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 240;
    clear_rect.h = 20;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.y = 464;
    clear_rect.h = 16;
    func_80074F44(&clear_rect, 0, 0, 0);

    bank = PE_LoadU32(0x801D11C8u) == 0u;
    PE_StoreU32(0x801D11C8u, bank);
    active = PE_LoadU32(0x801D11BCu + bank * 4u);
    PE_StoreU32(0x801D11C4u, active);

    (void)func_80074DC0(0);
    active = PE_LoadU32(0x801D11C4u);
    if ((int16_t)PE_LoadU16(active + 0x74u) > 0) {
        RECT blit;
        int32_t w;
        int32_t h;
        blit.x = (int16_t)PE_LoadU16(active + 0x70u);
        blit.y = (int16_t)PE_LoadU16(active + 0x72u);
        blit.w = (int16_t)PE_LoadU16(active + 0x74u);
        blit.h = (int16_t)PE_LoadU16(active + 0x76u);
        w = (int32_t)blit.x;
        blit.x = (int16_t)((w + w + w) >> 1);
        if (PE_LoadU32(0x801D11C8u) == 0u)
            blit.y = (int16_t)(blit.y + 240);
        h = (int32_t)blit.w;
        blit.w = (int16_t)((h + h + h) >> 1);
        (void)func_8007506C(&blit, active + 0x8080u);
        if (PE_Port_ShouldStop())
            return;
    }

    (void)func_80073A44(0);
    (void)func_80074A44(1);
    active = PE_LoadU32(0x801D11C4u);
    (void)func_80075424(active);
    if (PE_Port_ShouldStop())
        return;
    (void)func_800755F0(active + 0x5Cu);
}

int func_801909B4(void)
{
    pe_addr_t arena;
    pe_addr_t second;
    pe_addr_t environment0;
    pe_addr_t environment1;
    uint32_t saved_bit;
    RECT move_rect;
    RECT clear_rect;

    CopyGuestBytes(GA_DISPENV_COPY_0, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_COPY_1, GA_DISPENV_SOURCE_1, 0x14u);
    CopyGuestBytes(GA_DRAWENV_COPY_0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_COPY_1, GA_DRAWENV_SOURCE_1, 0x5Cu);

    /* 0x80190A90: retained in s4 for the later 0x80190D7C branch. */
    saved_bit = PE_LoadU8(0x800B0DCDu) & 1u;

    arena = PE_LoadU32(0x80011610u);
    second = arena + 0x1C080u;
    PE_StoreU32(0x800B0E50u, arena + 0x4080u);
    PE_StoreU32(0x801D11BCu, arena);
    PE_StoreU32(0x801D11C0u, second);
    PE_StoreU32(0x800B0E54u, second + 0x4080u);
    PE_StoreU32(0x800B0E38u, arena + 0x80u);
    PE_StoreU32(0x800B0E3Cu, second + 0x80u);

    func_8005E57C(1);
    func_8005C1EC(1);
    if (PE_Port_ShouldStop())
        return -1;
    func_80042538();
    func_80074D28(0);

    move_rect.x = 320;
    move_rect.y = 0;
    move_rect.w = 160;
    move_rect.h = 256; /* jal delay slot at 0x80190C0C */
    (void)func_8007512C(&move_rect, 0x2C0, 0);
    if (PE_Port_ShouldStop())
        return -1;

    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    environment0 = PE_LoadU32(0x801D11BCu);
    environment1 = PE_LoadU32(0x801D11C0u);
    (void)func_80074924(environment0, 0, 0, 320, 240);
    (void)func_80074924(environment1, 0, 240, 320, 240);
    (void)func_800749D8(environment0 + 0x5Cu, 0, 240, 320, 240);
    (void)func_800749D8(environment1 + 0x5Cu, 0, 0, 320, 240);

    PE_StoreU16(environment1 + 0x66u, 0u);
    PE_StoreU16(environment0 + 0x66u, 0u);
    PE_StoreU16(environment1 + 0x6Au, 240u);
    PE_StoreU16(environment0 + 0x6Au, 240u);
    PE_StoreU8(environment1 + 0x6Du, 1u);
    PE_StoreU8(environment0 + 0x6Du, 1u);
    PE_StoreU16(environment1 + 0x7Cu, 0u);
    PE_StoreU16(environment0 + 0x7Cu, 0u);
    PE_StoreU16(environment1 + 0x74u, 0u);
    PE_StoreU16(environment0 + 0x74u, 0u);

    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 480;
    clear_rect.h = 480;
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    if (PE_Port_SkipMovie()) {
        /* HOST_ADAPTED: skip logo fade, FMV001, and title. */
        (void)saved_bit;
        Stub_Record("func_801909B4_skip_movie_new_game", "HOST_ADAPTED");
        Trace_Direct("skip_movie_new_game_selector");
        return FinishTitleMenu(1);
    }

    if (PE_LoadU32(0x8009D1BCu) == 0u) {
        PE_StoreU32(0x8009D1BCu, 1u);
        (void)func_80190660();
        if (PE_Port_ShouldStop())
            return -1;
    }

    if (saved_bit != 0u) {
        int movie = func_80192CE8(1);
        unsigned i;

        if (PE_Port_ShouldStop())
            return -1;
        /* Retail: nonzero movie result waits 60 frames (skip pressed). */
        if (movie != 0) {
            for (i = 0u; i < 60u; i++) {
                func_800425DC();
                if (PE_Port_ShouldStop())
                    return -1;
                (void)func_80073A44(0);
            }
        }
        PostMovieRestoreEnvironments();
        if (PE_Port_ShouldStop())
            return -1;
    }

    /* 0x80191120: title present (func_8018F2F4), then freelist. */
    func_8018F2F4();
    if (PE_Port_ShouldStop())
        return -1;
    (void)func_80073A44(0);
    func_80074D28(1);

    {
        pe_addr_t node = GA_TITLE_NODE_POOL;
        pe_addr_t end = GA_TITLE_NODE_END;
        pe_addr_t next;

        while (node < end) {
            next = node + GA_TITLE_NODE_STRIDE;
            PE_StoreU32(node, next);
            node = next;
        }
        PE_StoreU32(node, 0u);
        PE_StoreU32(GA_TITLE_FREELIST, GA_TITLE_NODE_POOL);
        PE_StoreU32(0x801D137Cu, 0u);
        PE_StoreU32(0x801D1378u, 0u);
        PE_StoreU32(0x801D1374u, 0u);
        PE_StoreU32(0x801D1370u, 0u);
    }

    if (PE_Port_SkipOpeningMenu()) {
        Stub_Record("func_801909B4_skip_opening_menu", "HOST_ADAPTED");
        Trace_Direct("skip_opening_menu_after_title_present");
        return FinishTitleMenu(1);
    }

    /* 0x80191198: jal func_8018FBC0(1), install 0x8019319C at +0x14. */
    {
        pe_addr_t object = func_8018FBC0(1);

        PE_StoreU32(object + 0x14u, GA_TITLE_CALLBACK);
        PE_StoreU32(GA_TITLE_SAVED_BIT, saved_bit);
    }

    /* 0x801911C0: one title-loop body (bank flip, 425DC, 3EB04, 90064,
     * 8F468, list merge, present).  Retail branches back here while
     * D_801D1380+saved_bit < 1000; cut at that continue. */
    {
        uint32_t bank = PE_LoadU32(0x801D11C8u) == 0u ? 1u : 0u;
        pe_addr_t active = PE_LoadU32(0x801D11BCu + bank * 4u);

        PE_StoreU32(0x801D11C8u, bank);
        PE_StoreU32(0x801D11C4u, active);
    }

    PE_Port_SetCardTestEventHostReturn(1);
    func_800425DC();
    PE_Port_SetCardTestEventHostReturn(0);
    if (PE_Port_ShouldStop()) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_title_loop_425DC", "func_801909B4", 0x801911F0u,
            saved_bit, environment0, environment1, 0u);
        return -1;
    }

    /* jal 0x8003EB04 — only when retail libpad dispatch is already installed
     * (PadInit / test seed).  B54KR's RAM canary path leaves 9B738 unset and
     * therefore still cuts here without planting pad records. */
    if (PE_LoadU32(0x8009B738u) != 0x80084B20u) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_title_loop_3EB04", "func_801909B4", 0x801911F8u,
            saved_bit, environment0, environment1, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }

    func_8003EB04();
    if (PE_Port_ShouldStop()) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_title_loop_3EB04", "func_801909B4", 0x801911F8u,
            saved_bit, environment0, environment1, 0u);
        return -1;
    }

    func_80190064();
    if (PE_Port_ShouldStop()) {
        Bootstrap_ReturnVoid4Indirect(
            "func_80190064", "func_801909B4", 0x80190064u,
            saved_bit, environment0, environment1, 0u);
        return -1;
    }

    func_8018F468();
    if (PE_Port_ShouldStop()) {
        Bootstrap_ReturnVoid4Indirect(
            "func_8018F468", "func_801909B4", 0x8018F468u,
            saved_bit, environment0, environment1, 0u);
        return -1;
    }

    TitleListSweepAndMerge();
    TitleLoopPresent();
    if (PE_Port_ShouldStop()) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_title_present", "func_801909B4", 0x80191318u,
            saved_bit, environment0, environment1, 0u);
        return -1;
    }

    PE_StoreU32(GA_TITLE_SAVED_BIT,
                PE_LoadU32(GA_TITLE_SAVED_BIT) + saved_bit);
    if (PE_LoadU32(GA_TITLE_SAVED_BIT) >= 1000u) {
        Bootstrap_ReturnVoid4Indirect(
            "func_80074D28", "func_801909B4", 0x80191410u,
            saved_bit, environment0, environment1, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }

    Bootstrap_ReturnVoid4Indirect(
        "func_801909B4_title_main_loop", "func_801909B4", 0x801911C0u,
        saved_bit, environment0, environment1, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1;
}
