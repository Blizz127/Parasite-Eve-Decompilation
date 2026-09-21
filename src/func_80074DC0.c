/*
 * func_80074DC0 — display-slot dispatch with an optional debug-log call.
 *
 * VRAM 0x80074DC0 / file 0x655C0 / size 0x68 (26 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   if (D_8009574E >= 2)
 *       D_80095748(&D_80011884, a0);
 *   return D_80095744->f(0x3C)(a0);
 *
 * The `+0x3C` handler slot takes `a0`; the D_80095748/D_80011884 log vector
 * pair matches the sibling `func_80074D28`.
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern unsigned char D_8009574E;
extern int (*D_80095748)(char *, int);
extern struct D44b { char pad[0x3C]; unsigned int (*f)(int); } *D_80095744;
extern char D_80011884;

unsigned int func_80074DC0(int a0) {
    if (D_8009574E >= 2)
        D_80095748(&D_80011884, a0);
    return D_80095744->f(a0);
}
