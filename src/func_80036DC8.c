/* VRAM 0x80036DC8 / file 0x275C8 / size 0x30.
 * Three-call init sequence: 36DF8, 36E34, 36E58. */
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);
void func_80036DC8(void) {
    func_80036DF8();
    func_80036E34();
    func_80036E58();
}
