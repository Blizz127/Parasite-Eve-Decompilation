/* VRAM 0x8004F7D8 / file 0x3FFD8 / size 0x30. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x1E8 resolves to D_8009CF58. */
extern unsigned char *D_8009CF58;
void func_8005E8A4(int a0, int a1);
void func_80053648(unsigned char *state);

void func_8004F7D8(void) {
    func_8005E8A4(6, 6);
    func_80053648(D_8009CF58);
}
