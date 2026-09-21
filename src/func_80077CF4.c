extern int func_80077D30(int a0);

int func_80077CF4(int a0) {
    if (a0 < 0) {
        return -func_80077D30((-a0) & 0xFFF);
    }
    return func_80077D30(a0 & 0xFFF);
}
