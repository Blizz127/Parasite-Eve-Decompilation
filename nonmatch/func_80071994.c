/* VRAM 0x80071994 / file 0x62194 / size 0x30.
 * Pointer helper: base a0+8, optionally advanced by the word at a0+8 when
 * bit 3 of a0+4 is set; result +0xC. */
int func_80071994(int a0) {
    int *p = (int *)a0;
    if (p[1] & 8)
        return (a0 + 8) + p[2] + 0xC;
    return (a0 + 8) + 0xC;
}
