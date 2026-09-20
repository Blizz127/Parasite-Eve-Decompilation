/* VRAM 0x800900E4 / file 0x808E4 / size 0x24.
 * Advance a buffer cursor, latch the byte read before it into +0xB4 << 7. */
void func_800900E4(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(unsigned short *)(obj + 0xB4) = (unsigned short)(*(unsigned char *)cursor << 7);
}
