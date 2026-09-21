/* VRAM 0x8005267C / file 0x42E7C / size 0x48. */
extern int D_800B0E08;
extern void func_8006DF50(int, int, int, int, int);
void func_8005267C(void) {
    if (D_800B0E08 != 0) {
        func_8006DF50(D_800B0E08, 0x44E, 0x100, 0x80, 0x7F);
    }
}
