/*
 * func_8007DBC8 — pointer-global table read with an optional global shift.
 * VRAM 0x8007DBC8 / file 0x6E3C8 / size 0x3C (15 words).
 *
 * The value is a narrow `unsigned short` local so cc1 homes it in $a0
 * (`lhu a0,0(a0)` / `sllv v0,a0,v0`); D_8009B3FC is a pointer global and
 * D_8009B424 is `unsigned int` so the shift count is unsigned. era -O2 -G0.
 */
extern unsigned short *D_8009B3FC;
extern unsigned int D_8009B424;

int func_8007DBC8(unsigned int a0, int a1) {
    unsigned short v = D_8009B3FC[a0];

    if (a1 == -1)
        return v;
    return v << D_8009B424;
}
