extern int (*D_8009B738)(void);
extern void func_80083BB8(int a, int b);
void func_800828F4(int a0, int a1) {
    int r = D_8009B738();
    func_80083BB8(r, a1);
}
