extern void func_8007506C(int *a0, int *a1);

int *func_800718D0(int *a0) {
    int *s0 = 0;
    int *v0;
    if (a0[1] & 8) {
        s0 = a0 + 2;
        v0 = (int *)((char *)s0 + a0[2]);
    } else {
        v0 = a0 + 2;
    }
    func_8007506C(v0 + 1, v0 + 3);
    if (s0)
        func_8007506C(s0 + 1, s0 + 3);
    return v0 + 3;
}
