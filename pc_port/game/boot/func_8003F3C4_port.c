/*
 * PE-BTL38 — func_8003F3C4 field-tick named cut (translated
 * retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 227 words 0x8003F3C4..0x8003F750. Sole TEXT caller 1220C @
 * 0x800123D8. 1220C stores D1C4=D280 immediately before the
 * jal, so the 3F3E8 equality holds on entry.
 *
 * This cut is the live mailbox/fade sites only:
 *   jal 65400 @ 0x8003F4E8
 *   jal 35558 @ 0x8003F4F0
 *   jal 68E24 @ 0x8003F588 when (B0CD8&0x100)==0 and
 *     (B0CD8&0x200)==0
 *
 * 3F074, 3EB04, 6EC08, overlay 122040/121A00, 6E60C, 68CE0,
 * 37870, 661A4, E01BC, 661CC, 70E54, 66C7C, 73A44, 6A25C,
 * 6A0E8, 74F44, 74DC0, 87024, 3DFC8, 696F0 are not this cut.
 * Do not invent those bodies.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003F3C4(void)
{
    uint32_t bits;

    func_80065400();
    func_80035558_walk_cut();
    bits = PE_LoadU32(0x800B0CD8u);
    if ((bits & 0x100u) != 0u)
        return;
    if ((bits & 0x200u) != 0u)
        return;
    func_80068E24();
}
