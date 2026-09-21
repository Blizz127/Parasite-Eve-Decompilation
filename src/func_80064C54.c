/* VRAM 0x80064C54 / file 0x55454 / size 0x2C.
 * Tail wrapper: func_8005F354(func_8005DC4C(), D_8009D164) where D_8009D164
 * is the gp-relative 0x3F4($gp) word. era -O2 -G8. */
extern int D_8009D164;

int func_8005DC4C(void);
void func_8005F354(int value, int state);

void func_80064C54(void) {
    func_8005F354(func_8005DC4C(), D_8009D164);
}
