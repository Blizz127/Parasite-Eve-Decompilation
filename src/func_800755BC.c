/*
 * func_800755BC — reset a 0x5C-byte display record from the D_8009575C template.
 *
 * VRAM 0x800755BC / file 0x65DBC / size 0x34 (13 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   func_80071A34(a0, &D_8009575C, 0x5C);
 *   return a0;
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern int func_80071A34(void *a0, void *a1, int a2);
extern char D_8009575C;

void *func_800755BC(void *a0) {
    func_80071A34(a0, &D_8009575C, 0x5C);
    return a0;
}
