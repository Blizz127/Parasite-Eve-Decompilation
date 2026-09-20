/*
 * func_8008E7F4 — compare a record halfword against a1 and dispatch.
 * VRAM 0x8008E7F4 / file 0x7EFF4 / size 0x4C (19 words). era -O2 -G0.
 */
extern void func_8008E4E8(unsigned char *a0, int a1);
extern void func_8008E664(unsigned char *a0, int a1);
void func_8008E7F4(unsigned char *a0, int a1) {
    int v = *(short *)(a0 + 0xE2);
    if ((unsigned)v < (unsigned)a1) func_8008E4E8(a0, a1);
    else if ((unsigned)a1 < (unsigned)v) func_8008E664(a0, a1);
}
