void func_80083E50(unsigned char *a0, unsigned char a1) {
    a0[0x36] = 0x43;
    *(unsigned char **)(a0 + 0x2C) = a0 + 0x24;
    a0[0x24] = a1;
    a0[0x35] = 1;
}
