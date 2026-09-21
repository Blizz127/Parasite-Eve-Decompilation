/*
 * func_80075B4C — initialise a display-list node from a rectangular region.
 *
 * VRAM 0x80075B4C / file 0x6634C / size 0x38 (14 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   a0[3] = 2;
 *   *(int *)(a0 + 4) = func_800762BC(a1);
 *   *(int *)(a0 + 8) = 0;
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern int func_800762BC(int a0);

void func_80075B4C(unsigned char *a0, int a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_800762BC(a1);
    *(int *)(a0 + 8) = 0;
}
