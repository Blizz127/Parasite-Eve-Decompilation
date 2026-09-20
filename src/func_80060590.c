/* VRAM 0x80060590 / file 0x50D90 / size 0x34. era -O2 -G8.
 * gp base 0x8009CD70; +0x3B4 D_8009D124, +0x3B8 D_8009D128.
 * The local pointer keeps cc1 from folding the zero addend away: retail
 * keeps the load/store pair for D_8009D128 (redundant self-store). */
extern int D_8009D124;
extern int D_8009D128;
void func_800602D0(int value, int arg);

void func_80060590(int value) {
    int *p = &D_8009D128;
    func_800602D0(value, 4);
    D_8009D124 += 4;
    *p = D_8009D128;
}
