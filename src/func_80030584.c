/* Angle helper: ratan2 of shifted shorts vs vector, then +2048 as i16.
 * VRAM 0x80030584 / file 0x20D84 / size 0x44 (17 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 */
extern int func_80079FB4(int x, int z);

int func_80030584(unsigned char *a0, int *a1) {
    int x = ((int)*(short *)(a0 + 0xB4)) << 16;
    int z = ((int)*(short *)(a0 + 0xB8)) << 16;
    int r = func_80079FB4(x - a1[0], z - a1[2]);
    return (int)(short)(r + 2048);
}
