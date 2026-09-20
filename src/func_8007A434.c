/* VRAM 0x8007A434 / file 0x6AC34 / size 0x34.
 * Pointer-table lookup with 0x7 bound; fallback is the D_800119CC string. */
extern unsigned char *D_8009B05C[];
extern unsigned char D_800119CC[];

unsigned char *func_8007A434(unsigned int index) {
    index &= 0xFF;
    if (index >= 7)
        return D_800119CC;
    return D_8009B05C[index];
}
