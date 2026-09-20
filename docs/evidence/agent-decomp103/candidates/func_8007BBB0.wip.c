/* VRAM 0x8007BBB0 / file 0x6C3B0 / size 0x4C. */
extern int D_8009AFB8;
extern int D_8009AFB4;
extern int D_8009AFC8;
extern int D_8009AFC4;
extern void func_80073C94(void);
extern void func_80073CC4(int, void *);
extern void func_8007C13C(void);
void func_8007BBB0(void) {
    D_8009AFB8 = 0;
    D_8009AFB4 = 0;
    D_8009AFC8 = 0;
    D_8009AFC4 = 0;
    func_80073C94();
    func_80073CC4(2, func_8007C13C);
}
