void func_80077B34(unsigned char *a0, int a1) {
    if (a1 != 0) {
        a0[7] |= 1;
    } else {
        a0[7] &= 0xFE;
    }
}
