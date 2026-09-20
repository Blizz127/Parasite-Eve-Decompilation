/* VRAM 0x8005022C / file 0x40A2C / size 0x34. era -O2 -G8.
 * gp base 0x8009CD70; +0x184 D_8009CEF4, +0x1B0 D_8009CF20, +0x1E8 D_8009CF58. */
extern void *D_8009CEF4;
extern void *D_8009CF20;
extern void *D_8009CF58;
void func_80050AD8(void);
void func_800638D8(void *node, void (*callback)(void));

void func_8005022C(void *node) {
    D_8009CEF4 = node;
    D_8009CF20 = D_8009CF58;
    func_800638D8(node, func_80050AD8);
}
