/* VRAM 0x800762A0 / file 0x66AA0 / size 0x1C.
 * Pack a GPU primitive word: (a0 & 0x7FF) | 0xE5000000 | ((a1 & 0x7FF) << 11). */
int func_800762A0(int a0, int a1) {
    int hi = (a1 & 0x7FF) << 11;
    int lo = (a0 & 0x7FF) | 0xE5000000;
    return hi | lo;
}
