/* VRAM 0x80052634 / file 0x42E34 / size 0x48. */
extern int D_800B0E08;
extern void func_8006DF50(int, int, int, int, int);
void func_80052634(void) {
    if (D_800B0E08 != 0) {
        func_8006DF50(D_800B0E08, 0x44D, 0x100, 0x80, 0x7F);
    }
}
