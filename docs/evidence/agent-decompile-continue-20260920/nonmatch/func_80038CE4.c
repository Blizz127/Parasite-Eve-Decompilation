/* VRAM 0x80038CE4 / file 0x294E4 / size 0x28.
 * Two-stage byte-table lookup through pointer global D_80091A28:
 * base[base[(index & 0xFF) + 0x1D] + 4]. */
extern unsigned char *D_80091A28;

int func_80038CE4(unsigned int index) {
    unsigned char *base = D_80091A28;
    return base[base[(index & 0xFF) + 0x1D] + 4];
}
