/* VRAM 0x80018E84 / file 0x9684 / size 0x30.
 * Store two 16-bit fields read through a double indirection into the
 * D_800BD020 / D_800BD022 globals; always returns 1. */
extern unsigned short D_800BD020;
extern unsigned short D_800BD022;

int func_80018E84(int a0) {
    D_800BD020 = *(unsigned short *)(*(int *)(a0 + 0));
    D_800BD022 = *(unsigned short *)(*(int *)(a0 + 4));
    return 1;
}
