/* VRAM 0x8008FC78 / file 0x80478 / size 0x3C. */
int func_8008FC78(unsigned char **arg0) {
    int v = *(*arg0)++;
    if (v == 0) {
        v = 0x100;
    }
    *(short *)((char *)arg0 + 0x82) = v;
    *(short *)((char *)arg0 + 0xE6) = 0;
    *(short *)((char *)arg0 + 0x80) = 0;
    *(short *)((char *)arg0 + 0x84) = 1;
    return 1;
}
