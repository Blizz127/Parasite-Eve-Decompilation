/*
 * func_80075C94 — three-field descriptor writer.
 * VRAM 0x80075C94 / file 0x66494 / size 0x54 (21 words).
 * era -O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 (stack restore fills the
 * `jr ra` delay slot).
 */
extern int func_80076150(int a0, int a1, int a2);
extern int func_800762BC(int a0);
void func_80075C94(unsigned char *a0, int a1, int a2, int a3, int a4) {
    a0[3] = 2;
    *(int *)(a0 + 4) = func_80076150(a1, a2, a3 & 0xFFFF);
    *(int *)(a0 + 8) = func_800762BC(a4);
}
