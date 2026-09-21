/*
 * func_80075AE8 — reset a 0x14-byte display record from the D_800957B8 template.
 *
 * VRAM 0x80075AE8 / file 0x662E8 / size 0x34 (13 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   func_80071A34(a0, &D_800957B8, 0x14);
 *   return a0;
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern int func_80071A34(void *a0, void *a1, int a2);
extern char D_800957B8;

void *func_80075AE8(void *a0) {
    func_80071A34(a0, &D_800957B8, 0x14);
    return a0;
}
