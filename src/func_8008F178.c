/* VRAM 0x8008F178 / file 0x7F978 / size 0x38. */
extern char D_800B2900;
extern void func_8008F0D0(int, int *, int);
void func_8008F178(int arg0, int arg1) {
    int *p;
    *(short *)(arg0 + 0x5A) = arg1;
    p = (int *)((char *)&D_800B2900 + (arg1 << 6));
    func_8008F0D0(arg0, p, *p);
}
