/* VRAM 0x8008A02C / file 0x7A82C / size 0x3C.
 * Add (a1 - first word) to word 0 and word 1 of each 0x40-byte record. */
void func_8008A02C(unsigned char *a0, int a1, int a2) {
    int delta = a1 - *(int *)a0;
    unsigned char *p = a0 + 4;
    do {
        *(int *)a0 += delta;
        a0 += 0x40;
        *(int *)p += delta;
        p += 0x40;
        a2--;
    } while (a2 != 0);
}
