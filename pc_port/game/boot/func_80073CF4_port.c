/*
 * Phase 6E-B53F/B53G — complete Psy-Q DMA callback registration wrapper
 * func_80073CF4.
 *
 * Retail func_80073CF4 is 12 words / 0x30 bytes at
 * 0x80073CF4..0x80073D23 (exclusive end 0x80073D24).  It loads the libetc
 * jump-table pointer D_8009566C, loads field +0x04, invokes that guest
 * identity without changing a0/a1, and forwards the callee's v0 unchanged.
 * The first guard-passing ResetCallback installs func_800746A0 in that
 * field.  The native ResetCallback already collapses this SDK dispatch
 * table, so the only proven installed identity — the 32-bit guest address
 * 0x800746A0 — is bound directly here.  No native function pointer and no
 * guest-RAM callback mirror is introduced.
 *
 * B53F stopped at that backend.  B53G translates the complete 43-word
 * func_800746A0 callback-slot/DICR setter, so the wrapper is now complete:
 * it performs the indirect dispatch and forwards the full 32-bit
 * previous-handler return.  This path no longer requests a host stop and no
 * longer produces a fabricated registration result.
 *
 * Pre-install and dirty jump-table behaviour remains deliberately
 * unrepresented; no executable path reaches this wrapper before the
 * ResetCallback install chain runs.
 */

#include "psx_compat.h"
#include "game_port.h"

/* ResetCallback-installed jump-table field +0x04 (see b53f/b53g docs). */
#define GA_DMA_CALLBACK_SETTER 0x800746A0u

pe_addr_t func_80073CF4(uint32_t dma_channel, pe_addr_t handler)
{
    /* 0x80073D0C jalr v0 with a0/a1 unchanged; 0x80073D1C jr ra forwards
     * the callee's v0 with no truncation or normalisation. */
    return func_800746A0(dma_channel, handler);
}
