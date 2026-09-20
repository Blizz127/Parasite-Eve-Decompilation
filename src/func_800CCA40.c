/*
 * func_800CCA40 — decrement a byte, advance a halfword, arm a1[1] at 0x1E.
 * VRAM 0x800CCA40 / file 0xBD240 / size 0x38 (14 words). era -O2 -G0.
 */
void func_800CCA40(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 2;
    *(unsigned short *)(a2 + 4) += 0x28;
    if (*(signed char *)(a2 + 3) < 0x1E)
        a1[1] = 2;
}
