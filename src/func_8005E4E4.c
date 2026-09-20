/*
 * func_8005E4E4 — gp-guarded call result, bit 0x20 normalised to 0/1.
 * VRAM 0x8005E4E4 / file 0x4ECE4 / size 0x34. era -O2 -G8.
 */
extern int D_8009D0E8;

int func_8005E4E4(void) {
    int r = 0;
    if (D_8009D0E8) {
        unsigned int v = func_8005E038() & 0x20;
        r = (v > 0);
    }
    return r;
}
