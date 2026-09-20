/* VRAM 0x80090948 / file 0x81148 / size 0x28.
 * Advance a buffer cursor and latch the fetched byte into three u16 fields;
 * clear +0xD2. */
void func_80090948(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    unsigned short value;
    *(unsigned char **)obj = cursor + 1;
    value = *(unsigned char *)cursor;
    *(unsigned short *)(obj + 0xD2) = 0;
    *(unsigned short *)(obj + 0x58) = value;
    *(unsigned short *)(obj + 0x56) = value;
    *(unsigned short *)(obj + 0xD0) = value;
}
