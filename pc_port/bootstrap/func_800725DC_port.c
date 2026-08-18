/*
 * Phase 6E-A — func_800725DC: one-time static-init runner (PsyQ crt0 shape).
 *
 * Retail evidence (asm/disc1/5F3E4.s @ file 0x62DDC, 28 words / 0x70):
 *   t0 = D_80094538;
 *   if (t0 == 0) {
 *       D_80094538 = 1;
 *       s0 = &jtbl_80010000;        // init table at EXE load base
 *       s1 = 0;                     // entry count — materializes to 0
 *       if (s1) do { jalr *s0++; } while (--s1);
 *   }
 * The table count resolves to 0 in the retail binary, so the jalr loop is
 * dead: full retail semantics = set the guard once, invoke nothing.
 *
 * Contract: void(void); no arguments, no return value; sole side effect is
 * the D_80094538 guard (guest RAM).  Sole caller on the first-clear path:
 * func_8001220C (main, jal @ 0x80012224).  D_80094538 is referenced only by
 * func_800725DC and its sibling runner func_8007264C.
 *
 * Classification: HOST_ADAPTED — SDK crt0 behavior, faithfully reproducible
 * (this is not a constant-return bootstrap stub).
 */
#include "psx_compat.h"

#define GA_D_80094538  0x80094538u

void func_800725DC(void)
{
    if (PE_LoadU32(GA_D_80094538) == 0) {
        PE_StoreU32(GA_D_80094538, 1);
        /* jtbl_80010000 init table: 0 entries at retail — nothing to invoke */
    }
}
