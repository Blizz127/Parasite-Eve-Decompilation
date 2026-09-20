/* VRAM 0x80050308 / file 0x40B08 / size 0x34. era -O2 -G8.
 * gp base 0x8009CD70; gp+0x1A8 resolves to D_8009CF18. */
extern unsigned int D_8009CF18;
void func_8005EB64(unsigned int value);

void func_80050308(unsigned int index) {
    func_8005EB64(index + (D_8009CF18 ? 0x7C : 0x7F));
}
