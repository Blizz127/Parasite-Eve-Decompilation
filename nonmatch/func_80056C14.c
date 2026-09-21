/* VRAM 0x80056C14 / file 0x47414 / size 0x2C.
 * Bounds-checked read from the 32-byte-stride table at D_800A1E6E. */
extern unsigned short D_800A1E6E[];

unsigned short func_80056C14(unsigned int index) {
    unsigned short v = 0;
    if (index < 3)
        v = D_800A1E6E[index << 4];
    return v;
}
