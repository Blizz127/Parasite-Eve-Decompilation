/*
 * func_80075C94 — display-list node init (command + reset triplet).
 *
 * VRAM 0x80075C94 / file 0x66494 / size 0x54 (21 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   a0[3] = 2;
 *   *(int *)(a0 + 4) = func_80076150(a1, a2, a3 & 0xFFFF);
 *   *(int *)(a0 + 8) = func_800762BC(a4);
 *
 * `a4` arrives on the stack (offset 0x30 relative to the caller's frame) which
 * is the fifth outgoing argument; the 0xFFFF mask on a3 is retail's `andi`.
 *
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1; LINK_EXACT.
 */

extern int func_80076150(int a0, int a1, int a2);
extern int func_800762BC(int a0);

void func_80075C94(unsigned char *a0, int a1, int a2, int a3, int a4) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_80076150(a1, a2, a3 & 0xFFFF);
    *(int *)(a0 + 8) = func_800762BC(a4);
}
