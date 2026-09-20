/*
 * func_800404A8 [0x800404A8,0x800405A4), 63 words, asm/disc1/307CC.s.
 *
 * Inventory help "idle decay" helper.  Each call decays the signed watchdog
 * D_800A1844 by one.  For each of the two 0x418-byte cursor selector arrays
 * (base 0x800A0EDC and 0x800A12F4), a selector value of 2 or 3 reloads the
 * watchdog to 0xC and marks the matching slot (0x800A1848 / 0x800A184C).
 * If a marked slot's selector is not 4, the watchdog is clamped to at most 4
 * and both marks are cleared.  Returns 1 while the watchdog is still positive.
 *
 * Hand-transcribed from the retail assembly: there is no matching C leaf for
 * this function yet, so this is port behavior, not a verified match.  It is
 * needed by func_8004C608 help id 36 (see func_8004DF74_port.c).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define A_800404A8_TIMER   0x800A1844u
#define A_800404A8_SEL0    0x800A0EDCu
#define A_800404A8_SEL1    0x800A12F4u
#define A_800404A8_MARK0   0x800A1848u
#define A_800404A8_MARK1   0x800A184Cu

uint32_t func_800404A8(void)
{
    uint32_t slot;
    uint32_t offset = 0u;
    pe_addr_t mark = A_800404A8_MARK0;
    int32_t timer = (int32_t)PE_LoadU32(A_800404A8_TIMER);
    uint32_t dec = (timer > 0) ? 1u : 0u;      /* slt $v1,$zero,$v0 */

    PE_StoreU32(A_800404A8_TIMER, (uint32_t)timer - dec);

    /* Two selector arrays, byte selectors of 2 or 3 reload + mark. */
    for (slot = 0u; slot < 2u; slot++) {
        uint32_t byte = PE_LoadU8(A_800404A8_SEL0 + offset);
        uint32_t hit = ((byte - 2u) < 2u) ? 1u : 0u;   /* sltiu v0,v0,2 */
        if (hit) {
            PE_StoreU32(A_800404A8_TIMER, 0xCu);
            PE_StoreU32(mark, 1u);
        }
        mark += 4u;
        offset += 0x418u;
    }

    mark = A_800404A8_MARK0;
    if (PE_LoadU32(mark) != 0u &&
        PE_LoadU8(A_800404A8_SEL0) != 4u)
        goto done;
    if (PE_LoadU32(A_800404A8_MARK1) != 0u &&
        PE_LoadU8(A_800404A8_SEL1) != 4u)
        goto done;

    timer = (int32_t)PE_LoadU32(A_800404A8_TIMER);
    PE_StoreU32(A_800404A8_TIMER, (timer < 5) ? (uint32_t)timer : 4u);
    PE_StoreU32(A_800404A8_MARK1, 0u);
    PE_StoreU32(A_800404A8_MARK0, 0u);

done:
    return ((int32_t)PE_LoadU32(A_800404A8_TIMER) > 0) ? 1u : 0u;
}
