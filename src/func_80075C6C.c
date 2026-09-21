/*
 * func_80075C6C — set a rectangle-command display-list node's flag word.
 *
 * VRAM 0x80075C6C / file 0x6646C / size 0x28 (10 words).
 *
 * Retail (asm/disc1/654C8.s):
 *   a0[3] = 2;
 *   *(int *)(a0 + 4) = a1 ? 0xE6000001 : 0xE6000000;
 *   *(int *)(a0 + 8) = 0;    (the second store lands in the jr delay slot)
 *
 * era -O2 -G0; LINK_EXACT.
 */

void func_80075C6C(unsigned char *a0, int a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = a1 ? 0xE6000001u : 0xE6000000u;
    *(int *)(a0 + 8) = 0;
}
