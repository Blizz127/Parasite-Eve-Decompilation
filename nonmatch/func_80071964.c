/* VRAM 0x80071964 / file 0x62164 / size 0x30.
 * Pointer helper: base a0+8, optionally advanced by the word at a0+8 when
 * bit 3 of a0+4 is set; result +4. */
int func_80071964(int a0) {
    int *p = (int *)a0;
    if (p[1] & 8)
        return (a0 + 8) + p[2] + 4;
    return (a0 + 8) + 4;
}
