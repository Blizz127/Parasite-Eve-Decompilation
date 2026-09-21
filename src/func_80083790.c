int func_80083790(unsigned char *a0) {
    unsigned int v = ((*(unsigned char *)(a0 + 0xE3) + 1) >> 1) << 2;
    unsigned int b = *(unsigned char *)(a0 + 0xE9);
    unsigned int off = ((b * 5 + 3) & 0xFFC) + 4;
    return (int)((unsigned char *)(v + off) + *(unsigned int *)(a0 + 0xEC));
}
