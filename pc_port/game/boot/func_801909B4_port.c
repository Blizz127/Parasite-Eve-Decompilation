/*
 * Phase 6E-B54K-Q — func_801909B4 prefix through the first MoveImage.
 *
 * Retail overlay body: [0x801909B4,0x801918F8), 977 words.
 * Implemented pre-call prefix: [0x801909B4,0x80190C08), 149 words.
 * The boundary pair at 0x80190C08 is jal func_8007512C / sh h,30(sp).
 * Native captures all three MoveImage arguments and the complete local RECT,
 * then stops.  No VRAM move, later overlay call, or scheduler state is
 * fabricated.
 */
#include "psx_compat.h"
#include "game_port.h"

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
extern void func_80042538(void);

static void CopyGuestBytes(pe_addr_t destination, pe_addr_t source,
                           uint32_t size)
{
    memcpy(PE_Translate(destination, size),
           PE_TranslateConst(source, size), size);
}

int func_801909B4(void)
{
    pe_addr_t arena;
    pe_addr_t second;
    RECT move_rect;

    CopyGuestBytes(GA_DISPENV_COPY_0, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_COPY_1, GA_DISPENV_SOURCE_1, 0x14u);
    CopyGuestBytes(GA_DRAWENV_COPY_0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_COPY_1, GA_DRAWENV_SOURCE_1, 0x5Cu);

    /* 0x80190A90: retained for the later 0x80190D7C branch. */
    (void)(PE_LoadU8(0x800B0DCDu) & 1u);

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
    (void)Bootstrap_ReturnInt4Indirect(
        "func_8007512C", "func_801909B4", -1, 0x8007512Cu,
        0u, 0x2C0u, 0u, 0u, &move_rect, sizeof(move_rect));
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1; /* retail's retained s2 value */
}
