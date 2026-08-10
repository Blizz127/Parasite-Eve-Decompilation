/*
 * Phase 6E-B53F — execution-proven prefix of the Psy-Q DMA callback
 * registration wrapper func_80073CF4.
 *
 * Retail func_80073CF4 is 12 words / 0x30 bytes at
 * 0x80073CF4..0x80073D23 (exclusive end 0x80073D24).  It loads the libetc
 * jump-table pointer D_8009566C, loads field +0x04, invokes that guest
 * identity without changing a0/a1, and forwards the callee's v0 unchanged.
 * The first guard-passing ResetCallback installs func_800746A0 in that
 * field.  The native ResetCallback already collapses this SDK dispatch
 * table, so the proven installed identity is represented here as a 32-bit
 * guest address, never as a native function pointer or guest-RAM callback
 * mirror.
 *
 * func_800746A0 is a separate 43-word callback-slot/DICR setter and remains
 * unresolved in B53F.  Consequently this prefix performs no callback-table,
 * DICR, DMA, ring, or queue-pump mutation before exposing that boundary.
 */

#include "psx_compat.h"
#include "game_port.h"

#define GA_DMA_CALLBACK_SETTER 0x800746A0u

pe_addr_t func_80073CF4(uint32_t dma_channel, pe_addr_t handler)
{
    int result = Bootstrap_ReturnInt4Indirect(
        "func_800746A0", "func_80073CF4", 0,
        GA_DMA_CALLBACK_SETTER,
        dma_channel, handler, 0u, 0u, NULL, 0u);

    /* The indirect retail callee has not returned.  Non-strict execution
     * therefore stops at the same boundary instead of continuing into ring
     * publication with a fabricated callback-registration result. */
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return (pe_addr_t)(uint32_t)result;
}
