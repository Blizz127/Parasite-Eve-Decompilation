/*
 * Phase 6E-B32 — func_800614AC: packed-color interpolation leaf.
 *
 * Retail body: 36 instructions / 0x90 bytes,
 * 0x800614AC..0x80061538 (exclusive end 0x8006153C), file offset
 * 0x51CAC, live split asm/disc1/51CAC.s.  The complete transcription is
 * independently checked and executed by tools/b32_oracle.py.
 *
 * The leaf masks the input to 24 bits, averages the adjacent color bytes,
 * packs the result with the retail saturation branches, stores the masked
 * source at D_8009D14C and the packed result at D_8009D150, and returns the
 * packed result.  It has no callees or platform activity.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

#define GA_614AC_SOURCE 0x8009D14Cu /* retail $gp + 0x3DC */
#define GA_614AC_RESULT 0x8009D150u /* retail $gp + 0x3E0 */

int func_800614AC(int a0)
{
    uint32_t packed = (uint32_t)a0 & 0x00FFFFFFu;
    int32_t high = (int32_t)((uint32_t)a0 >> 16) & 0xFF;
    int32_t middle = ((int32_t)((uint32_t)a0 >> 8)) & 0xFF;
    int32_t low = (int32_t)(uint32_t)a0 & 0xFF;
    int32_t high_middle = (high + middle) >> 1;
    int32_t high_low = (high + low) >> 1;
    int32_t middle_low = (middle + low) >> 1;
    uint32_t upper;
    uint32_t result;

    PE_StoreU32(GA_614AC_SOURCE, packed);

    if (high_middle < 0x100)
        upper = (uint32_t)high_middle;
    else
        upper = 0xFFu;
    if (high_low < 0x100)
        upper |= (uint32_t)high_low << 8;
    else
        upper |= 0xFF00u;
    if (middle_low < 0x100)
        result = (uint32_t)middle_low << 16;
    else
        result = 0x00FF0000u;

    result |= upper;
    PE_StoreU32(GA_614AC_RESULT, result);
    return (int)result;
}
