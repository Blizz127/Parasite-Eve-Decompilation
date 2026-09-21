/* VRAM 0x800509A8 / file 0x411A8 / size 0x38. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x1A4 resolves to D_8009CF14. */
extern unsigned int D_8009CF14;
void func_8005EB58(int value);
void func_80064C54(unsigned int value);

void func_800509A8(unsigned int index) {
    func_8005EB58(0);
    func_80064C54(D_8009CF14 + index);
}
