/* VRAM 0x800501C8 / file 0x409C8 / size 0x3C. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x184 resolves to D_8009CEF4. */
extern void *D_8009CEF4;
void func_8005100C(void);
void func_800638D8(void *node, void (*callback)(void));
void func_8005EB58(int value);
void func_8005EB64(int value);

void func_800501C8(void *node) {
    D_8009CEF4 = node;
    func_800638D8(node, func_8005100C);
    func_8005EB58(1);
    func_8005EB64(0x68);
}
