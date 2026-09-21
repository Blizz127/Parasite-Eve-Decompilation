/*
 * func_80087050 — wait for / test the low bit of a status word.
 * VRAM 0x80087050 / file 0x77850 / size 0x40 (16 words).
 *
 * D_8009D2E0 must be volatile: without it cc1 hoists the second load out of
 * the loop and reuses a stale value. era -O2 -G0.
 */
extern volatile unsigned int D_8009D2E0;

int func_80087050(int a0) {
    if (a0 == 0) {
        while (D_8009D2E0 & 1)
            ;
        return 0;
    }
    return D_8009D2E0 & 1;
}
