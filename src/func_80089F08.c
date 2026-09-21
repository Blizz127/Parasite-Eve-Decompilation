/* VRAM 0x80089F08 / file 0x7A510 / size 0x1C.
 * 16-byte-stride struct field read through pointer global D_8009B3FC. */
extern unsigned char *D_8009B3FC;

void func_80089F08(int index, unsigned short *out) {
    *out = *(unsigned short *)(D_8009B3FC + index * 16 + 0xC);
}
