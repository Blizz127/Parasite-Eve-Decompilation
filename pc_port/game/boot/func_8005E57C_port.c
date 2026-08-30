/*
 * Phase 6E-B54K-Q — complete three-word scalar setter func_8005E57C.
 *
 * Retail 0x8005E57C..0x8005E588 stores a0 at gp+0x3B0
 * (D_8009D120) and returns.  The matching lane independently identifies
 * the same C leaf.  Classification: complete translated retail function.
 */
#include "psx_compat.h"

void func_8005E57C(int value)
{
    PE_StoreU32(0x8009D120u, (uint32_t)value);
}
