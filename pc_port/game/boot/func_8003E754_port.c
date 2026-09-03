/*
 * Phase 6E-A — func_8003E754: video init (DISPENV/DRAWENV double-buffer setup).
 *
 * ROM: asm/disc1/2EF54.s @ file 0x2EF54, 114 words, frame 0x38.
 * Caller: func_8003E610 with (w=0x140, h=0xE0).
 * Classification: 1 (translated game logic); all SDK callees are now real
 * (pe_libgpu.c, host_framebuffer.c).
 *
 * Retail ordering notes (reproduced deliberately):
 *   - isrgb24 = 1 is stored to both DISPENVs BEFORE the SetDefDispEnv calls;
 *     SetDefDispEnv then zeroes +0x11 again.  Final retail state: isrgb24 = 0.
 *   - screen overrides (screen.x = 0, screen.y = 8, screen.h = h; screen.w
 *     stays 0) land between the SetDefDispEnv and SetDefDrawEnv calls.
 *   - DRAWENV overrides (tpage = 0, dtd = 1, dfe = 0, isbg = 1, r0/g0/b0 = 0)
 *     land after both SetDefDrawEnv calls.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

void func_8003E754(int w, int h)
{
    RECT rc = { 0, 0, 0x3FF, 0x200 };

    func_80074A44(0);                       /* ResetGraph */
    func_80074BB8(0);                       /* SetGraphDebug */
    func_80074D28(0);                       /* SetDispMask */
    func_80074F44(&rc, 0, 0, 1);            /* ClearImage */

    /* isrgb24 = 1 on both DISPENVs (overwritten to 0 by SetDefDispEnv) */
    PE_StoreU8(0x800BCE91u, 1);
    PE_StoreU8(0x800BCEA5u, 1);

    func_800749D8(0x800BCE80u, 0, h, w, h); /* SetDefDispEnv buf0 */
    func_800749D8(0x800BCE94u, 0, 0, w, h); /* SetDefDispEnv buf1 */

    /* screen overrides: x = 0, y = 8, h = h (w left 0) */
    PE_StoreU16(0x800BCE8Au, 8);            /* buf0 screen.y */
    PE_StoreU16(0x800BCE9Eu, 8);            /* buf1 screen.y */
    PE_StoreU16(0x800BCE88u, 0);            /* buf0 screen.x */
    PE_StoreU16(0x800BCE9Cu, 0);            /* buf1 screen.x */
    PE_StoreU16(0x800BCE8Eu, (uint16_t)h);  /* buf0 screen.h */
    PE_StoreU16(0x800BCEA2u, (uint16_t)h);  /* buf1 screen.h */

    func_80074924(0x800BCDC8u, 0, 0, w, h); /* SetDefDrawEnv buf0 */
    func_80074924(0x800BCE24u, 0, h, w, h); /* SetDefDrawEnv buf1 */

    /* DRAWENV overrides: tpage = 0, dtd = 1, dfe = 0, isbg = 1, rgb = 0 */
    PE_StoreU16(0x800BCDDCu, 0);            /* buf0 tpage */
    PE_StoreU16(0x800BCE38u, 0);            /* buf1 tpage */
    PE_StoreU8(0x800BCDDEu, 1);             /* buf0 dtd */
    PE_StoreU8(0x800BCE3Au, 1);             /* buf1 dtd */
    PE_StoreU8(0x800BCDDFu, 0);             /* buf0 dfe */
    PE_StoreU8(0x800BCE3Bu, 0);             /* buf1 dfe */
    PE_StoreU8(0x800BCDE0u, 1);             /* buf0 isbg */
    PE_StoreU8(0x800BCE3Cu, 1);             /* buf1 isbg */
    PE_StoreU8(0x800BCDE1u, 0);             /* buf0 r0 */
    PE_StoreU8(0x800BCE3Du, 0);             /* buf1 r0 */
    PE_StoreU8(0x800BCDE2u, 0);             /* buf0 g0 */
    PE_StoreU8(0x800BCE3Eu, 0);             /* buf1 g0 */
    PE_StoreU8(0x800BCDE3u, 0);             /* buf0 b0 */
    PE_StoreU8(0x800BCE3Fu, 0);             /* buf1 b0 */

    D_8009CDDC = 0;
    func_800755F0(0x800BCE80u);            /* PutDispEnv(buf0) */
}
