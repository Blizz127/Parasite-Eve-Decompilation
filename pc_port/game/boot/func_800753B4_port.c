/*
 * Phase 6E-DRW1 — func_800753B4 DrawOTag, translated retail.
 *
 * Retail: [0x800753B4,0x80075424), 28 words, asm/disc1/654C8.s:531-562.
 * Debug name "DrawOTag(%08x)...\n" at D_80011928.  No matching src/ C
 * (native translation, not a decomp leaf; same standard as OTC1).
 *
 * Body, ROM order: prologue (sp-0x18, save ra/s0); s0 = ot; level gate
 * (print through *D_80095748 only at level >= 2, then fall through);
 * jtb dispatch: a0 = jtb[6], v0 = jtb[2], jalr with (a1 = ot carried
 * over from the print setup, a2 = 0, a3 = 0); epilogue; return the
 * dispatch result.  The worker identity passes through unchecked exactly
 * as retail passes it: func_80076C34 validates the worker at its jalr
 * (76664 / 76B98 exact, anything else an indirect boundary).
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_GPU_DEBUG_LEVEL 0x8009574Eu
#define GA_GPU_PRINT_FN    0x80095748u
#define GA_GPU_JTB_PTR     0x80095744u
#define GA_GPU_PRINT_NAME  0x80011928u   /* "DrawOTag(%08x)...\n" */
#define GA_GPU_DISPATCH    0x80076C34u   /* jtb[2] */

int func_800753B4(pe_addr_t ot)
{
    pe_addr_t jtb;
    pe_addr_t target;

    /* Debug print only at level >= 2; retail falls through afterwards. */
    if (PE_LoadU8(GA_GPU_DEBUG_LEVEL) >= 2u) {
        pe_addr_t print_target = PE_LoadU32(GA_GPU_PRINT_FN);
        Bootstrap_ReturnVoid4Indirect(
            "func_80071A74", "func_800753B4", print_target,
            GA_GPU_PRINT_NAME, ot, 0u, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    /* jtb[2] dispatch with (jtb[6], ot, 0, 0).  The worker passes through
     * unchecked exactly as retail passes it; the dispatcher validates it.
     * PE_LoadU32 aborts on unmapped addresses, so an unrepresentable jtb
     * reports target 0 rather than faulting. */
    jtb = PE_LoadU32(GA_GPU_JTB_PTR);
    if (!PE_RangeIsRam(jtb, 0x20u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076C34", "func_800753B4", 0, 0u,
            0u, ot, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    target = PE_LoadU32(jtb + 8u);
    if (target != GA_GPU_DISPATCH) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076C34", "func_800753B4", 0, target,
            PE_LoadU32(jtb + 0x18u), ot, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    return func_80076C34(PE_LoadU32(jtb + 0x18u), ot, 0, 0);
}
