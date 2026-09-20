/* func_80067B40 — store selector bytes, then masked-OR 0x400 into D_800BCF88
 * through a local pointer (retail keeps one base register). era_o2_g0. */
extern int D_800BCF88;
extern unsigned char D_800BCFFA;
extern unsigned char D_800BCFFB;
int func_80067B40(unsigned char a0) {
    int *p = &D_800BCF88;
    D_800BCFFA = a0;
    D_800BCFFB = 0;
    *p = (*p & ~0xC00) | 0x400;
    return 0;
}
