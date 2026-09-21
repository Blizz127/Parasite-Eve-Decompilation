/*
 * func_800753B4 — reset via the shared log template, then drive the display slot.
 *
 * VRAM 0x800753B4 / file 0x65BB4 / size 0x70 (28 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   if (D_8009574E >= 2)
 *       D_80095748(&D_80011928, a0);
 *   D_80095744->f(0x8)(D_80095744->v[6], a0, 0, 0);
 *
 * `D_80095744` is a pointer global; retail keeps the loaded base in `$v0` and
 * reads both the `+0x8` handler and the `+0x18` argument word from it.
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, int);
extern unsigned int *D_80095744;
extern char D_80011928;

void func_800753B4(int a0) {
    unsigned int *v0;

    if (D_8009574E >= 2)
        D_80095748(&D_80011928, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[6], a0, 0, 0);
}
