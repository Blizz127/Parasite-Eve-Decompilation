/* VRAM 0x80078C94 / file 0x69494 / size 0x24.
 * Copy three words from src+0/4/8 to dst+0x14/0x18/0x1C; returns dst. */
unsigned char *func_80078C94(unsigned char *dst, unsigned char *src) {
    *(unsigned int *)(dst + 0x14) = *(unsigned int *)(src + 0);
    *(unsigned int *)(dst + 0x18) = *(unsigned int *)(src + 4);
    *(unsigned int *)(dst + 0x1C) = *(unsigned int *)(src + 8);
    return dst;
}
