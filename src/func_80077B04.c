void func_80077B04(unsigned char *a0, int a1) {
    if (a1 != 0) {
        a0[7] |= 2;
    } else {
        a0[7] &= 0xFD;
    }
}
