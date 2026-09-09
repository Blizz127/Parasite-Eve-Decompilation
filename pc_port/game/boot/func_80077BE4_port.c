/*
 * SetPolyFT4 header (matching leaf func_80077BE4, not matching src/ here).
 * 5 words at 0x80077BE4: p[3]=12, p[7]=60 (code 0x3C).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_80077BE4(pe_addr_t p)
{
    PE_StoreU8(p + 3, 12);
    PE_StoreU8(p + 7, 60);
}
