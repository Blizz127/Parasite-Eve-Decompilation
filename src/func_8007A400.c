/* VRAM 0x8007A400 / file 0x6AC00 / size 0x34.
 * Pointer-table lookup with 0x1C bound; fallback is the D_800119CC string. */
extern unsigned char *D_8009AFDC[];
extern unsigned char D_800119CC[];

unsigned char *func_8007A400(unsigned int index) {
    index &= 0xFF;
    if (index >= 0x1C)
        return D_800119CC;
    return D_8009AFDC[index];
}
