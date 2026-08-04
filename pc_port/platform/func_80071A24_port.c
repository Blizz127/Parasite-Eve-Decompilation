/*
 * Phase 6E-B21a — func_80071A24: BIOS SysEnqIntRP trampoline.
 *
 * Raw body: 3 words / 0xC, exe 0x80071A24–0x80071A2F, file 0x62224,
 * live split asm/disc1/5F3E4.s:3436–3440; all 3 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * BIOS operation:
 *   addiu $t2, $zero, 0xA0   → BIOS B0 table vector
 *   jr $t2                    → enter kernel
 *   addiu $t1, $zero, 0x28   → B0(0Ah) = SysEnqIntRP
 *
 * SysEnqIntRP(addr, size) — System Enqueue Interrupt Request Packet.
 * Registers a guest-memory event-packet buffer with the PS1 kernel.
 * Returns 0 on success; writes nothing to guest RAM (kernel-side only).
 *
 * Host adaptation:
 *   No PS1 kernel is available.  The call returns 0 (success) with no
 *   guest-memory effects.  The eight flag bytes at offsets within the
 *   packet region are set afterward by func_80064964.
 *
 * Sole call site: func_80064964 @0x80064974 ($a0=0x800A3060,
 * $a1=0x120, addiu delay slot).
 *
 * Classification: 2 — known Psy-Q/BIOS behavior (host-adapted as
 * deterministic success return).
 */
#include "psx_compat.h"

int func_80071A24(pe_addr_t addr, int size)
{
    (void)addr;
    (void)size;
    return 0;   /* SysEnqIntRP returns 0 on success */
}
