/* RNG gate: if record inner+0x10 bit 16 is set and RNG%100 < rec+0x22,
 * store 9000 at rec+0x10.
 * VRAM 0x80030640 / file 0x20E40 / size 0xA0 (40 words).
 * gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * ROM `lui $v1,1` is 0x10000, not bit 0 (`andi 1` was the leftover).
 * Second D_8009D278 load is into $v1: %100 clobbers $a0, so the store
 * reloads the global rather than reusing rec.
 */
extern unsigned char *D_8009D278;
extern int func_80071A54(void);

void func_80030640(void) {
    unsigned char *rec;
    unsigned int *inner;
    unsigned short thresh;
    int rnd;

    rec = D_8009D278;
    inner = *(unsigned int **)(rec + 0x68);
    if ((inner[4] & 0x10000) == 0)
        return;
    thresh = *(unsigned short *)(rec + 0x22);
    rnd = func_80071A54();
    if ((rnd % 100) < (int)thresh) {
        unsigned char *rec2 = D_8009D278;
        *(short *)(rec2 + 0x10) = 9000;
    }
}
