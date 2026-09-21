/* VRAM 0x80089F28 / file 0x7A728 / size 0x28.
 * Mirror two u16 values into pointer-global fields +0x184/+0x186 and the
 * static D_8009B3A4[0..1]. */
extern unsigned char *D_8009B3FC;
extern unsigned short D_8009B3A4[];

void func_80089F28(unsigned short a, unsigned short b) {
    *(unsigned short *)(D_8009B3FC + 0x184) = a;
    *(unsigned short *)(D_8009B3FC + 0x186) = b;
    D_8009B3A4[0] = a;
    D_8009B3A4[1] = b;
}
