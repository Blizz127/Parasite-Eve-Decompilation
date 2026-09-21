/* VRAM 0x8007E594 / file 0x6ED94 / size 0x30.
 * Zero word 0, byte 4, four bytes 5..8, then words 0xC/0x10/0x14. */
void func_8007E594(unsigned char *p) {
    int i;
    *(unsigned int *)(p + 0) = 0;
    p[4] = 0;
    for (i = 3; i >= 0; i--)
        p[5 + i] = 0;
    *(unsigned int *)(p + 0xC) = 0;
    *(unsigned int *)(p + 0x10) = 0;
    *(unsigned int *)(p + 0x14) = 0;
}
