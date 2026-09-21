/*
 * func_8007506C — reset a record, then push two fields out the display slot.
 *
 * VRAM 0x8007506C / file 0x6586C / size 0x60 (24 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   func_80074E28(&D_800118D4, a0);                 ; 0x14-byte template copy
 *   D_80095744->f(0x20)(D_80095744->v[0x20], a0, 8, a1);
 *
 * `D_80095744` is a pointer global (`extern unsigned int *`): retail loads it
 * with `lui`+`lw` and keeps the single base in `$v0`, then reads the `+0x8`
 * handler and the `+0x20` argument word from that one base.
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern unsigned int *D_80095744;
extern char D_800118D4;
extern void func_80074E28(char *a0, int a1);

void func_8007506C(int a0, int a1) {
    unsigned int *v0;

    func_80074E28(&D_800118D4, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[8], a0, 8, a1);
}
