void func_80083C20(unsigned char *a0) {
    unsigned int value = *(unsigned int *)(a0 + 0x20);
    a0[0x36] = 0x4D;
    a0[0x35] = 6;
    *(unsigned int *)(a0 + 0x2C) = value;
}
