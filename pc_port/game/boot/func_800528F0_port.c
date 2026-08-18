/*
 * Phase 6E-B18 — func_800528F0: PRNG table generator.
 *
 * Raw body: 143 words / 0x23C, exe 0x800528F0–0x80052B2B, file 0x430F0,
 * live split asm/disc1/42FC8.s:104–260; all 143 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site: func_800527C8 @0x800527F4 (nop delay slot, no
 * arguments, return ignored).  First unresolved callee in B17, now
 * translated.
 *
 * Operation (ROM order):
 *   1. Read D_800A76A4 (timer tick, 60 Hz) → divide by 60 via
 *      unsigned reciprocal (multu ×0x88888889, mfhi, srl 5).
 *   2. Seed a local 32-bit LCG: s(n+1) = lo32(s(n) × 0x5D588B65) + 1.
 *   3. Generate 17 32-bit words: each accumulates the MSB of 32
 *      successive LCG states, shifted in from the right.
 *   4. XorShift expand to 521 words: W[i] = (W[i-17]<<23)^(W[i-16]>>9)^W[i-1]
 *      with a pre-iteration at i=16 using W[16],W[0],W[15].
 *   5. Extract low byte of each word to D_800A1B90[0..520].
 *   6–8. Self-referential XOR mixing (two passes):
 *        output[0..31]  ^= output[489..520]   (via D_800A1D79 overlap)
 *        output[32..520] ^= output[0..488]    (via D_800A1B70 overlap)
 *      D_800A1D79 = D_800A1B90 + 489, D_800A1B70 = D_800A1B90 - 32.
 *   9. Store 0x208 (520) at D_8009D038 ($gp+0x2C8).
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include <stdint.h>

#define GA_TIMER       0x800A76A4u
#define GA_OUT         0x800A1B90u
#define GA_CNT         0x8009D038u
#define TABLE_BYTES    521
#define XORA_OFF       489

void func_800528F0(void)
{
    uint32_t w[TABLE_BYTES];
    uint32_t state;
    int i, bit;

    /* 1. Seed from timer tick / 60. */
    {
        uint32_t tick = PE_LoadU32(GA_TIMER);
        state = (uint32_t)(((uint64_t)tick * 0x88888889uLL) >> 32) >> 5;
    }

    /* 2–3. Generate 17 words via local LCG. */
    for (i = 0; i < 17; i++) {
        uint32_t word = 0;
        for (bit = 0; bit < 32; bit++) {
            state = (uint32_t)((uint64_t)state * 0x5D588B65uLL) + 1u;
            word = (word >> 1) | (state & 0x80000000u);
        }
        w[i] = word;
    }

    /* 4. XorShift expansion. */
    w[16] = (w[16] << 23) ^ (w[0] >> 9) ^ w[15];
    for (i = 17; i < TABLE_BYTES; i++)
        w[i] = (w[i - 17] << 23) ^ (w[i - 16] >> 9) ^ w[i - 1];

    /* 5. Extract low bytes. */
    for (i = 0; i < TABLE_BYTES; i++)
        PE_StoreU8(GA_OUT + (uint32_t)i, (uint8_t)(w[i] & 0xFFu));

    /* 6–8. Self-referential XOR mixing — two identical passes.
     *    D_800A1D79 = GA_OUT + 489  →   output[0..31] ^= output[489..520]
     *    D_800A1B70 = GA_OUT - 32   →   output[32..520] ^= output[0..488] */
    for (int pass = 0; pass < 2; pass++) {
        for (i = 0; i < 32; i++) {
            uint8_t b = PE_LoadU8(GA_OUT + (uint32_t)i);
            b ^= PE_LoadU8(GA_OUT + (uint32_t)(XORA_OFF + i));
            PE_StoreU8(GA_OUT + (uint32_t)i, b);
        }
        for (i = 32; i < TABLE_BYTES; i++) {
            uint8_t b = PE_LoadU8(GA_OUT + (uint32_t)i);
            b ^= PE_LoadU8(GA_OUT + (uint32_t)(i - 32));
            PE_StoreU8(GA_OUT + (uint32_t)i, b);
        }
    }

    /* 9. Store byte count. */
    PE_StoreU32(GA_CNT, 0x208u);
}
