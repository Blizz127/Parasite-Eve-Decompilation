/* VRAM 0x8008F4E8 / file 0x7FCE8 / size 0x2C.
 * Bump the byte cursor at +0, set bits 0-1 of the +0xF4 flags, and store the
 * consumed byte shifted left 8 into +0x6C. */
void func_8008F4E8(int a0) {
    unsigned char *cursor = *(unsigned char **)a0;
    *(unsigned char **)a0 = cursor + 1;
    *(int *)(a0 + 0xF4) = *(int *)(a0 + 0xF4) | 3;
    *(unsigned short *)(a0 + 0x6C) = (unsigned short)(*(unsigned char *)cursor << 8);
}
