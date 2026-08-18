/*
 * Phase 6E-A — func_8003E944: save-manager bring-up (boot path).
 *
 * ROM: asm/disc1/2EF54.s (12 words): two calls, no frame.
 *   func_800844E4(&D_800BE9A0, &D_800BE9A0 + 0x22)
 *   func_80082534()
 * Classification: 1 (translated game logic); callees real in pe_save.c.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

void func_8003E944(void)
{
    func_800844E4(0x800BE9A0u, 0x800BE9A0u + 0x22u);
    func_80082534();
}
