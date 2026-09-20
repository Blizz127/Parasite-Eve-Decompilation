/* VRAM 0x800762A0 / file 0x66AA0 / size 0x1C.
 * Pack a 0xE5 texture-page word from two 11-bit fields. */
int func_800762A0(unsigned int a0, unsigned int a1) {
    return (int)(0xE5000000u | (a0 & 0x7FFu) | ((a1 & 0x7FFu) << 11));
}
