/*
 * func_80075C04 — initialise a display-list node from a point pair.
 *
 * VRAM 0x80075C04 / file 0x66404 / size 0x40 (16 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   a0[3] = 2;
 *   *(int *)(a0 + 4) = func_800762A0(a1[0], a1[1]);
 *   *(int *)(a0 + 8) = 0;
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern int func_800762A0(int a0, int a1);

void func_80075C04(unsigned char *a0, short *a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_800762A0(a1[0], a1[1]);
    *(int *)(a0 + 8) = 0;
}
