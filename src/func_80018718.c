/* VRAM 0x80018718 / file 0x8F18 / size 0x3C.
 * Store 1/0 through *a0 by D_800A76C4 bit 2; return 1. */
extern unsigned int D_800A76C4;
int func_80018718(int **a0) {
    if (D_800A76C4 & 4u)
        **a0 = 1;
    else
        **a0 = 0;
    return 1;
}
