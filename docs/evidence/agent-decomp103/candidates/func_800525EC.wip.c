/* VRAM 0x800525EC / file 0x42DEC / size 0x48. */
extern int D_800B0E08;
extern void func_8006DF50(int, int, int, int, int);
void func_800525EC(void) {
    if (D_800B0E08 != 0) {
        func_8006DF50(D_800B0E08, 0x44C, 0x100, 0x80, 0x7F);
    }
}
