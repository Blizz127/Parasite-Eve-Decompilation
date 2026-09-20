/* VRAM 0x80038CE4 / file 0x294E4 / size 0x28.
 * Double index into the byte table at *D_80091A28. */
extern unsigned char *D_80091A28;

unsigned char func_80038CE4(int index) {
    unsigned char *base = D_80091A28;
    unsigned char *rec = base + (index & 0xFF);
    unsigned char step = *(unsigned char *)(rec + 0x1D);
    return *(unsigned char *)(base + step + 4);
}
