extern unsigned char D_800A34B0[];
extern void func_80072714(void);
extern void func_8007E1F4(int a, unsigned char *p);
extern void func_80072724(void);
int func_8007E0C0(void) {
    func_80072714();
    func_8007E1F4(1, D_800A34B0);
    func_80072724();
    return 1;
}
