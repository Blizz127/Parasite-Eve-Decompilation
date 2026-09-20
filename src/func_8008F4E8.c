/* func_8008F4E8 - VRAM 0x8008F4E8, file 0x7FCE8, size 0x2C. */
int func_8008F4E8(int a0) {
    unsigned char *p = *(unsigned char **)a0;
    unsigned int flags;
    unsigned int b;
    *(unsigned int *)a0 = (unsigned int)(p + 1);
    flags = *(unsigned int *)(a0 + 0xF4);
    b = *p;
    flags |= 3u;
    b <<= 8;
    *(unsigned int *)(a0 + 0xF4) = flags;
    *(unsigned short *)(a0 + 0x6C) = (unsigned short)b;
    return (int)flags;
}
