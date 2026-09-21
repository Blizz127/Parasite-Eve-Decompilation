/*
 * func_800750CC — reset a record, then push two fields out the display slot.
 *
 * VRAM 0x800750CC / file 0x658CC / size 0x60 (24 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   func_80074E28(&D_800118E0, a0);
 *   D_80095744->f(0x1C)(D_80095744->v[0x1C], a0, 8, a1);
 *
 * Twin of matched `func_8007506C` (different template and slot: `+0x1C`
 * instead of `+0x20`).
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern unsigned int *D_80095744;
extern char D_800118E0;
extern void func_80074E28(char *a0, int a1);

void func_800750CC(int a0, int a1) {
    unsigned int *v0;

    func_80074E28(&D_800118E0, a0);
    v0 = D_80095744;
    (*(void (**)(int, int, int, int))(v0 + 2))(v0[7], a0, 8, a1);
}
