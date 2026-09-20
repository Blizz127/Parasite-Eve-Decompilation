/* VRAM 0x8004FC3C / file 0x4043C / size 0x44. */
extern int func_80054288(void);
extern int func_800556E8(int);
extern void func_8004324C(int);
void func_8004FC3C(int arg0) {
    if (arg0 < func_80054288()) {
        func_8004324C(func_800556E8(arg0));
    }
}
