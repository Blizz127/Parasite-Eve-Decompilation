/* func_8008F784 - VRAM 0x8008F784, file 0x7FF84, size 0x38.
 * Advance the byte cursor, publish ((byte + 0x40) & 0xFF) << 8 at +0x76,
 * set bits 0/1 of +0xF4, and clear +0x78. */
void func_8008F784(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned int b;
    *(unsigned int *)a0 = (unsigned int)(p + 1);
    b = *p;
    *(unsigned short *)(a0 + 0x78) = 0;
    *(unsigned int *)(a0 + 0xF4) = *(unsigned int *)(a0 + 0xF4) | 3u;
    *(unsigned short *)(a0 + 0x76) = (unsigned short)(((b + 0x40) & 0xFFu) << 8);
}
