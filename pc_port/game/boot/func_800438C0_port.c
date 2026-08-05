/*
 * Phase 6E-B28 — func_800438C0: masked-state setter.
 *
 * Matched C leaf (5EH, era -O2 -G8).  VRAM 0x800438C0, size 0x20
 * (8 words).  $gp-relative store to D_8009CEF0 ($gp+0x180).
 *
 * Masks arg with 0x1FF; if result is 0, stores 0x1FF instead.
 * Returns 0.
 *
 * Classification: 1 — translated retail logic (matched C).
 */
#include "psx_compat.h"

#define GA_438C0_STATE  0x8009CEF0u  /* $gp+0x180 */

int func_800438C0(int arg0)
{
    int m = arg0 & 0x1FF;
    if (m == 0)
        m = 0x1FF;
    PE_StoreU32(GA_438C0_STATE, (uint32_t)m);
    return 0;
}
