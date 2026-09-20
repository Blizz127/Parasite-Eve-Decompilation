/* VRAM 0x80082DBC / file 0x735BC / size 0x44. */
extern void func_80072714(void);
extern void func_80073C84(int, int);
extern char D_800A5AB0;
extern void func_8007E1F4(int, void *);
extern void func_80072724(void);
void func_80082DBC(void) {
    func_80072714();
    func_80073C84(3, 1);
    func_8007E1F4(2, &D_800A5AB0);
    func_80072724();
}
