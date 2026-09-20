/* VRAM 0x80021054 / file 0x11854 / size 0x2C.
 * Return -1 when the record flags keep bit 16, else the gp-relative signed
 * byte D_8009CE3C (0xCC($gp)).  era -O2 -G8.  D_8009D278 is declared as an
 * incomplete array so its incomplete type emits no `.extern <size>` and the
 * load stays absolute, while the scalar byte becomes gp-relative. */
extern int *D_8009D278[];
extern signed char D_8009CE3C;

int func_80021054(void) {
    int v;

    if (*(int *)((char *)D_8009D278[0] + 0x4C) & 0x10000) {
        v = -1;
    } else {
        v = D_8009CE3C;
    }
    return v;
}
