/* VRAM 0x800192DC / file 0x9ADC / size 0x30.
 * func_8006FC18(**a0, D_8009D2F0, 0); return 1. */
extern unsigned char *D_8009D2F0;
extern void func_8006FC18(int a0, unsigned char *a1, int a2);
int func_800192DC(int **a0) {
    func_8006FC18(**a0, D_8009D2F0, 0);
    return 1;
}
