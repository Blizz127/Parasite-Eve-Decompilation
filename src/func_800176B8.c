/* VRAM 0x800176B8 / file 0x7EB8 / size 0x28.
 * OR the caller's word into field 0x98 of the D_8009D2F0 object; return 1. */
extern int *D_8009D2F0;

int func_800176B8(int **arg) {
    D_8009D2F0[0x98 / 4] |= *arg[0];
    return 1;
}
