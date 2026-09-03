/*
 * Phase 6E-CDQ2d — func_8010BD4C (overlay, 0x8010BD4C..0x8010BE2C):
 * RLE frame decoder.  Pure guest-RAM worker, no calls.
 * Transcribed in MV1b, verified on the real B54KY stream (2239
 * ops, full 69632-byte frame), parked on delivery, relanded here
 * with the E0 delivery pump.
 *
 * Outer loop over the source at 0x8010CBFC, datum-driven:
 * - datum < 0xF0, v1 == 0: literal run — copy (datum+1) bytes
 *   from the source (BDB4 copy loop; the bltz is never taken:
 *   a1 = datum & 0xFF >= 0).
 * - datum < 0xF0, v1 != 0: back-copy — copy (datum+1) bytes from
 *   (dst - v1) (BD8C loop, same dead bltz).
 * - datum == 0xF0: v1 = 0 (via the beq delay slot).
 * - datum > 0xF0: v1 = lbu[a3++] extension byte, then
 *   v1 = ((datum << 8) | ext) + 0xFFFF0F01 (wraps mod 2^32).
 * The loop exits when v1 == 0xF00 — i.e. the terminator pair
 * FF FF (0xFFFF + 0xFFFF0F01 wraps to 0xF00).  a1 = 4 lands in
 * the bne delay slot on both arms.  The count argument is dead:
 * retail clobbers a1 with the first datum (andi) before any use.
 * Both copy loops advance the pointer in the bgez delay slot,
 * even on the exiting byte.
 *
 * Exit runs a 34812-halfword XOR-delta pass from dst+8
 * (a1 = 4 .. 0x8800 against the 0x87FF limit; the a0 += 2 delay
 * executes on the exiting pass too, into dead a0).
 */

#include "psx_compat.h"
#include "pe_sdk.h"

void func_8010BD4C(pe_addr_t dst, uint32_t count)
{
    uint32_t v1 = 0u;
    uint32_t a3 = 0x8010CBFCu;
    uint32_t a2 = dst;
    const uint32_t t2 = 0xF0u;
    const uint32_t t0 = 0xFFFF0F01u;
    const uint32_t t1 = 0xF00u;
    uint32_t a1;
    (void)count;

    for (;;) {
        uint32_t v0 = PE_LoadU8(a3);
        a1 = v0 & 0xFFu;
        if (a1 >= 0xF0u) {
            a3++;
            if (a1 == t2) {
                v1 = 0u;
            } else {
                v1 = PE_LoadU8(a3);
                a3++;
                v1 = ((a1 << 8) | v1) + t0;
            }
        } else {
            a3++;
            if (v1 != 0u) {
                if ((int32_t)a1 >= 0) {
                    for (;;) {
                        v0 = PE_LoadU8(a2 - v1);
                        a1--;
                        PE_StoreU8(a2, (uint8_t)v0);
                        a2++;
                        if ((int32_t)a1 < 0)
                            break;
                    }
                }
            } else {
                if ((int32_t)a1 >= 0) {
                    for (;;) {
                        v0 = PE_LoadU8(a3);
                        a3++;
                        a1--;
                        PE_StoreU8(a2, (uint8_t)v0);
                        a2++;
                        if ((int32_t)a1 < 0)
                            break;
                    }
                }
            }
        }
        a1 = 4u;
        if (v1 != t1)
            continue;
        break;
    }
    {
        uint32_t a1x = 4u;
        uint32_t a0x = dst + 8u;
        for (;;) {
            uint32_t x0 = PE_LoadU16(a0x);
            uint32_t x1 = PE_LoadU16(a0x - 8u);
            a1x++;
            PE_StoreU16(a0x, (uint16_t)(x0 ^ x1));
            a0x += 2u;
            if (0x87FFu < a1x)
                break;
        }
    }
}
