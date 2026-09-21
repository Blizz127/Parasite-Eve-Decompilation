/* VRAM 0x80026FD0 / file 0x177D0 / size 0x28.
 * If the signed byte D_8009D2B0 is nonzero, write 0x80 (unsigned) and -8
 * (signed) to the two gp-relative bytes D_8009CE68 (0xF8($gp)) and
 * D_8009CE6C (0xFC($gp)).
 * D_8009D2B0 is declared as an incomplete array so its load stays absolute;
 * the two destination bytes are complete scalars -> gp-relative. era -O2 -G8. */
extern signed char D_8009D2B0[];
extern unsigned char D_8009CE68;
extern signed char D_8009CE6C;

void func_80026FD0(void) {
    if (D_8009D2B0[0] != 0) {
        D_8009CE68 = 0x80;
        D_8009CE6C = -8;
    }
}
