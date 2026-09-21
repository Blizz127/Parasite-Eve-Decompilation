/* VRAM 0x800744D4 / file 0x64CD4 / size 0x4C. */
extern int D_800956C0;
extern int *D_800956BC;
extern void func_8007474C(int *, int);
extern void func_80073CC4(int, void *);
extern void func_80074520(void);
extern void func_800746A0(void);
int func_800744D4(void) {
    func_8007474C(&D_800956C0, 8);
    *D_800956BC = 0;
    func_80073CC4(3, func_80074520);
    return (int)func_800746A0;
}
