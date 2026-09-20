/* func_80076B20 - VRAM 0x80076B20, file 0x67320, size 0x24.
 * Publish a0 through the D_80095854 pointer and tag D_800A3348[a0>>24]. */
extern unsigned int D_80095854;
extern unsigned char D_800A3348[];

void func_80076B20(unsigned int a0) {
    *(unsigned int *)D_80095854 = a0;
    D_800A3348[a0 >> 24] = (unsigned char)a0;
}
