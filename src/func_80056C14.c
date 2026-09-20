extern unsigned short D_800A1E6E[];
unsigned int func_80056C14(unsigned int a0) {
    unsigned int r;
    if (a0 < 3)
        r = D_800A1E6E[a0 << 4];
    else
        r = 0;
    return r;
}
