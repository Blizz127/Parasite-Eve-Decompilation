/* VRAM 0x80016FE0 / file 0x77E0 / size 0x38.
 * Store 0/1 through *a0 based on D_8009D2E8 bit 0; return 1. */
extern unsigned int D_8009D2E8;
int func_80016FE0(int **a0) {
    if (D_8009D2E8 & 1u)
        **a0 = 0;
    else
        **a0 = 1;
    return 1;
}
