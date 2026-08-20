/* Angle between two actors: ratan2 of +0x28/+0x30, wrap to [0,4096).
 * VRAM 0x800305C8 / file 0x20DC8 / size 0x78 (30 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 * Callee of func_8001F814 (hit-react).
 */
extern int func_80079FB4(int x, int z);

int func_800305C8(unsigned int *a0, unsigned char *a1) {
    register int ang asm("$3");
    int wrap;
    int base;

    ang = 2048 - func_80079FB4((int)a0[10] - (int)((unsigned int *)a1)[10],
                               (int)a0[12] - (int)((unsigned int *)a1)[12]);
    ang = (int)(short)ang;
    ang += (int)*(short *)(a1 + 0x3A);
    wrap = ang;
    if (ang < 0)
        wrap = ang + 4095;
    base = (wrap >> 12) << 12;
    return (int)(short)(ang - base);
}
