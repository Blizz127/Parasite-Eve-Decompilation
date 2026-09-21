/* VRAM 0x80017294 / file 0x7A94 / size 0x28.
 * D_8009CE00 (0x90($gp)) = field 0x9C of the object pointed to by the global
 * D_8009D2F0, plus twice the word at **arg0; return 1.
 * D_8009D2F0 is declared as an incomplete pointer array so it stays absolute
 * (retail lui/lw) while the 4-byte store is gp-relative. era -O2 -G8. */
extern int *D_8009D2F0[];
extern unsigned int D_8009CE00;

int func_80017294(int **arg0) {
    D_8009CE00 = *(int *)((char *)D_8009D2F0[0] + 0x9C) + (**arg0 << 1);
    return 1;
}
