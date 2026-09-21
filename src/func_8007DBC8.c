/*
 * func_8007DBC8 — table lookup, optionally shifted by a global shift count.
 *
 * VRAM 0x8007DBC8 / file 0x6E3C8 / size 0x3C (15 words).
 * ROM: asm/disc1/6D874.s.
 *
 * Retail:
 *   unsigned short v = D_8009B3FC[a0];   // lhu a0,0(a0): value homed in $a0
 *   if (a1 == -1) return v;
 *   return v << D_8009B424;
 * The value must be a narrow `unsigned short` local so cc1 homes it in $a0
 * (`lhu a0,0(a0)` / `sllv v0,a0,v0` / `move v0,a0`); an `unsigned int` local
 * lands in $v1 and picks up a redundant `move`. D_8009B3FC is a pointer global
 * (one loaded base register), and D_8009B424 is `unsigned int` so the `sllv`
 * count is unsigned. The `-1` literal is materialised once.
 */
extern unsigned short *D_8009B3FC;
extern unsigned int D_8009B424;

int func_8007DBC8(unsigned int a0, int a1) {
    unsigned short v = D_8009B3FC[a0];

    if (a1 == -1)
        return v;
    return v << D_8009B424;
}
