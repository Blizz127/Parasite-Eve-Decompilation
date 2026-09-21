/* VRAM 0x80018E84 / file 0x9684 / size 0x30.
 * Store two double-dereferenced words into D_800BD020 / D_800BD022; return 1.
 * Volatile keeps retail's load-store-load-store order (no hoisting). */
extern volatile unsigned short D_800BD020;
extern volatile unsigned short D_800BD022;

int func_80018E84(int **arg) {
    D_800BD020 = *arg[0];
    D_800BD022 = *arg[1];
    return 1;
}
