/* VRAM 0x80076B44 / file 0x67344 / size 0x14.
 * Byte-table getter: D_800A3348[a0] with no index scaling. */
extern unsigned char D_800A3348[];

unsigned char func_80076B44(int index) {
    return D_800A3348[index];
}
