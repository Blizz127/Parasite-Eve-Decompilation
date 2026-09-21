/* VRAM 0x8008B040 / file 0x7B840 / size 0x44. */
extern void func_8008AE94(int);
extern int D_8009D22C;
void func_8008B040(int arg0) {
    int v;
    func_8008AE94(arg0);
    v = *(int *)(arg0 + 0x10);
    if (v != 0) {
        v = v - 1;
    }
    D_8009D22C = v;
}
