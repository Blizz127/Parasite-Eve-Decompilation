/*
 * Phase 6E-B54I — func_800719E4: BIOS B(38h) CD-mode trampoline.
 *
 * Raw body: 3 words / 0xC, exe 0x800719E4–0x800719EF, file 0x621E4;
 * all 3 instruction words verified exact against the SHA-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 * It is the first entry of the retail BIOS-call trampoline block
 * (0x800719E4..0x80071A83) whose A-table siblings include the already
 * ported func_80071A24 = A(28h) bzero.
 *
 * BIOS operation:
 *   addiu $t2, $zero, 0x00B0   → BIOS B-function table vector
 *   jr $t2                      → enter kernel
 *   addiu $t1, $zero, 0x0038   → B(38h) = CD mode set
 *
 * The B(38h) identity is already established in this tree by pe_libcd.c
 * (func_8006E6D4's mode-mismatch branch): "func_800719E4(1) — BIOS
 * B(38h) CD mode set: collapsed no-op", with the CD-layer justification
 * that the retail path is never taken at boot (D_8009B590 == D_800B0DD4
 * == 0).
 *
 * Caller census (executable-wide, this rung): twelve jal sites.  One is
 * the CD-layer mode set with a0=1 (func_8006E758, already collapsed in
 * pe_libcd.c).  The other eleven are the packet-length fail paths of the
 * primitive add/sort wrappers func_800370DC / func_80037140 and their
 * func_800373xx siblings, all reached only when func_80077CB4 returns -1
 * (compound packet length budget of 17 words exceeded), all passing
 * a0 = -1 in the jal delay slot (`li a0, -1`).  With retail data the
 * canonical first append is len 6 (or 5 for the tile twin), far below the
 * cap, so the path is not taken on the proven boot prefix.
 *
 * Guest-visible effect: none — B(38h) touches BIOS-internal CD state
 * only.  All twelve retail call sites discard $v0 (verified: every jal
 * is followed directly by the epilogue or by state reloads).  The native
 * port therefore performs the call boundary with no guest mutation and
 * returns 0.
 *
 * Classification: 2 — known BIOS operation collapsed with recorded
 * justification (same ruling as pe_libcd.c).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

uint32_t func_800719E4(uint32_t mode)
{
    /* B(38h) CD mode set: BIOS-internal only; no guest-RAM effect.
     * Return value unconsumed by all twelve retail callers. */
    (void)mode;
    return 0;
}
