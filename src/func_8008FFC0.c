/* VRAM 0x8008FFC0 / file 0x807C0 / size 0x24.
 * Advance a buffer cursor, latch the byte read before it into +0xA6 << 8. */
void func_8008FFC0(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(unsigned short *)(obj + 0xA6) = (unsigned short)(*(unsigned char *)cursor << 8);
}
