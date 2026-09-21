int func_800762A0(int a0, int a1) {
    int y = (a1 & 0x7FF) << 11;
    int x = (a0 & 0x7FF) | 0xE5000000;
    return y | x;
}
