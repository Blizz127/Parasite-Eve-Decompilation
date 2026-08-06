/*
 * Phase 6E-B31 — func_80042CC4: retail byte-ramp initializer.
 *
 * Retail body: 31 instructions / 0x7C bytes,
 * 0x80042CC4..0x80042D3C (exclusive end 0x80042D40), file offset
 * 0x334C4, live split asm/disc1/334C4.s.  The independent B31 oracle
 * transcribes and verifies every word before executing its own model.
 *
 * The function is a leaf.  It clears the first byte of D_800A1878, then
 * repeatedly interpolates the next byte from the current byte while the
 * signed byte/threshold comparison succeeds.  The final byte count is
 * written to $gp+0x170 (D_8009CEE0).  The caller's a2/a3 are overwritten
 * before either is used; no host pointer or hardware provider is involved.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

#define GA_42CC4_TABLE 0x800A1878u
#define GA_42CC4_COUNT 0x8009CEE0u /* retail gp 0x8009CD70 + 0x170 */

void func_80042CC4(int a0, int a1)
{
    uint32_t base = GA_42CC4_TABLE;
    uint32_t a2;
    uint32_t a3;
    uint32_t t0;

    PE_StoreU8(base, 0u);
    a2 = base;
    a3 = 0x100u - (uint32_t)a0;
    t0 = base + 0xFu;

    /* The initial branch's delay slot shifts the color base by 8. */
    uint32_t color = (uint32_t)a0 << 8;
    if (a2 < t0) {
        do {
            uint32_t byte = PE_LoadU8(a2);
            if (!((int32_t)byte < (int32_t)a1))
                break;
            uint32_t product = (uint32_t)((int64_t)(int32_t)a3 *
                                          (int64_t)(int32_t)byte);
            uint32_t next = (color + product) >> 8;
            PE_StoreU8(a2 + 1u, (uint8_t)next);
            a2 += 1u;
        } while (a2 < t0);
    }

    PE_StoreU32(GA_42CC4_COUNT, (a2 - base) + 1u);
}
