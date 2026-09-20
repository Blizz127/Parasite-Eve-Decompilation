/* VRAM 0x80083790 / file 0x73F90 / size 0x38. */
unsigned char *func_80083790(unsigned char *arg0) {
    int a = *(unsigned char *)(arg0 + 0xE3);
    int b = *(unsigned char *)(arg0 + 0xE9);
    unsigned char *c = *(unsigned char **)(arg0 + 0xEC);
    int x = ((a + 1) >> 1) << 2;
    int y = ((b << 2) + b + 3) & 0xFFC;
    y += 4;
    return c + x + y;
}
