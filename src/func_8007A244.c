/* VRAM 0x8007A244 / file 0x6AA44 / size 0x60.
 * Reset a block of absolute globals around a func_8007C444(0, D_800C20C4)
 * call, returning its result.  era -O2 -G0. */
extern int D_800C20C4;
extern int D_800BE9EC;
extern int D_800BE9E4;
extern int D_800BE998;
extern int D_800B89F4;
extern int D_800B0CD0;
extern short D_800A8018;
extern int D_800A5D54;
extern int func_8007C444();

int func_8007A244(void) {
    int r;
    D_800BE9EC = 0;
    D_800BE9E4 = 0;
    D_800BE998 = 0;
    D_800B89F4 = 0;
    r = func_8007C444(0, D_800C20C4);
    D_800B0CD0 = 0;
    D_800A8018 = 0;
    D_800A5D54 = 0;
    return r;
}
