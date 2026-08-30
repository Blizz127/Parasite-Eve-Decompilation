/*
 * Phase 6E-B54K-Q — complete func_80042538 state reset.
 *
 * Retail 0x80042538..0x800425DC clears the 0x830-byte record block at
 * D_800A0ED4, writes the two -1 sentinels, and clears thirteen scalar
 * owners.  Classification: complete translated retail function.
 */
#include "psx_compat.h"

void func_80042538(void)
{
    (void)func_80071A24(0x800A0ED4u, 0x830u);
    PE_StoreU32(0x800A12F8u, 0xFFFFFFFFu);
    PE_StoreU32(0x800A0EE0u, 0xFFFFFFFFu);
    PE_StoreU32(0x800A183Cu, 0u);
    PE_StoreU32(0x800A1840u, 0u);
    PE_StoreU32(0x800A1844u, 0u);
    PE_StoreU32(0x800A184Cu, 0u);
    PE_StoreU32(0x800A1848u, 0u);
    PE_StoreU32(0x800A1838u, 0u);
    PE_StoreU32(0x800A1854u, 0u);
    PE_StoreU32(0x800A1860u, 0u);
    PE_StoreU32(0x800A1864u, 0u);
    PE_StoreU32(0x800A1868u, 0u);
    PE_StoreU32(0x800A1704u, 0u);
    PE_StoreU32(0x800A185Cu, 0u);
    PE_StoreU32(0x800A186Cu, 0u);
}
