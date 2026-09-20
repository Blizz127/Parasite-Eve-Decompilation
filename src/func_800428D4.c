/* VRAM 0x800428D4 / file 0x330D4 / size 0x3C.
 * Clear a 1048-byte-stride entry's state byte at D_800A0ED5. */
extern unsigned int D_800A1860;
extern unsigned char D_800A0ED5[];
void func_800428D4(void) {
    unsigned int idx = D_800A1860 - 1;
    D_800A0ED5[idx * 1048] = 0xD;
}
