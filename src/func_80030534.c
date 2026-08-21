/* 2D distance helper: (dx)^2+(dz)^2 then jal already-C func_8005186C.
 * VRAM 0x80030534 / file 0x20D34 / size 0x50 (20 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 */
extern int func_8005186C(int);

int func_80030534(unsigned char *a0, unsigned char *a1) {
    int dx = (int)*(short *)(a0 + 0x268) - (int)*(short *)(a1 + 0x2A);
    int dz = (int)*(short *)(a0 + 0x26C) - (int)*(short *)(a1 + 0x32);
    return func_8005186C(dx * dx + dz * dz);
}
