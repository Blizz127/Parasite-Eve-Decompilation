/*
 * Phase 6E-B54K-R/Y — func_801909B4 through MoveImage, display setup, the
 * one-time overlay-local initializer, and its saved-bit branch.
 *
 * Retail overlay body: [0x801909B4,0x801918F8), 977 words.
 * Implemented caller path: [0x801909B4,0x80190D8C), 246 words through the
 * positive-arm call. MoveImage traverses the retail dispatcher and exact
 * one-packet GP0(80h) worker.
 * func_80190660 now completes its 480-frame loop.  Disc 1's saved bit then
 * enters func_80192CE8(1); the saved-bit-zero arm remains the exact named
 * structural cut at 0x80191120.
 */
#include "psx_compat.h"
#include "game_port.h"
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
extern int func_80190660(void);
extern int func_80192CE8(int index);

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
        /* HOST_ADAPTED dev entry (--skip-movie): skip the 480-frame logo
         * fade, the opening FMV (func_80192CE8), and the untranslated
         * title/menu tail.  Return the New-Game selector so
         * func_8006E9A0(1) publishes 0xA80830C8 and the next main
         * dispatch is the field tick.  Retail only reaches that after
         * FMV001 + the title Confirm; none of those ran here.  Execute
         * its original exit before returning to the New-Game caller. */
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
    if (saved_bit == 0u) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_80191120_cut", "func_801909B4", 0x80191120u,
            saved_bit, environment0, environment1, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }

    return func_80192CE8(1); /* retail retains 92CE8 result (s3) */
}
