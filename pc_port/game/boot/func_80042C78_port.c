/*
 * Phase 6E-B30 — func_80042C78: proven initialization prefix.
 *
 * Retail body: 16 instructions / 0x40 bytes,
 * 0x80042C78..0x80042CB4 (exclusive end), file offset 0x33478,
 * live split asm/disc1/33478.s.  The complete transcription is checked and
 * executed by tools/b30_oracle.py.
 *
 * func_80042CC4 remains unresolved.  This function preserves the five
 * preceding retail stores, calls that dependency with the exact post-delay
 * arguments, and commits the final store only if the dependency returns.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"
#include "pe_bootstrap.h"

void func_80042C78(void)
{
    PE_StoreU32(0x8009CED8u, 0u); /* $gp + 0x168 */
    PE_StoreU32(0x8009CEE0u, 0u); /* $gp + 0x170 */
    PE_StoreU32(0x8009CEE4u, 0u); /* $gp + 0x174 */
    PE_StoreU32(0x8009CEDCu, 0x20u); /* $gp + 0x16C */

    /* jal func_80042CC4; delay slot establishes a0=0x90, a1=0xFF. */
    Bootstrap_ReturnVoid("func_80042CC4", "func_80042C78");

    PE_StoreU32(0x8009CEECu, 0x48u); /* $gp + 0x17C */
}
