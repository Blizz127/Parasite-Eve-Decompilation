/* VRAM 0x80050B48 / file 0x41348 / size 0x4C. */
extern int func_80054288(void);
extern int func_800556E8(int);
extern int func_8005DC9C(int);
extern void func_8005F27C(int);
void func_80050B48(int arg0) {
    if (arg0 < func_80054288()) {
        func_8005F27C(func_8005DC9C(func_800556E8(arg0) + 0xEB));
    }
}
