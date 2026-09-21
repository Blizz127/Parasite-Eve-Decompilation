/*
 * func_800CCA78 — twin of func_800CCA40 with a 6/0xB4 step.
 * VRAM 0x800CCA78 / file 0xBD278 / size 0x38 (14 words). era -O2 -G0.
 */
void func_800CCA78(int a0, unsigned char *a1, unsigned char *a2) {
    *(unsigned char *)(a2 + 3) -= 6;
    *(unsigned short *)(a2 + 4) += 0xB4;
    if (*(signed char *)(a2 + 3) < 0x1E)
        a1[1] = 2;
}
