/* VRAM 0x800858B0 / file 0x760B0 / size 0x38.
 * Bounds-checked 16-byte-stride u16 table read through pointer global
 * D_8009B7D0; index masked to 16 bits, out-of-range returns 0. */
extern unsigned char *D_8009B7D0;

unsigned short func_800858B0(unsigned int index) {
    int i = index & 0xFFFF;
    if (i < 3)
        return *(unsigned short *)(D_8009B7D0 + i * 16);
    return 0;
}
