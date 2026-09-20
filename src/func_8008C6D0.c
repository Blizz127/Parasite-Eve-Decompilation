/*
 * func_8008C6D0 — store a word, then OR bit 1-0 across a 0x18-record table.
 * VRAM 0x8008C6D0 / file 0x7CED0 / size 0x3C (15 words). era -O2 -G0.
 */
extern unsigned char D_800B8BB4[];
extern unsigned int D_8009D2B8;
void func_8008C6D0(unsigned char *a0) {
    unsigned int i = 0;
    unsigned char *p = D_800B8BB4;
    D_8009D2B8 = *(unsigned int *)(a0 + 4);
    do {
        *(unsigned int *)p |= 3;
        i++;
        p += 0x11C;
    } while (i < 0x18);
}
