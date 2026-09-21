/*
 * func_8005E518 — gp-guarded call result, bit 0x5000 normalised to 0/1.
 * VRAM 0x8005E518 / file 0x4ED18 / size 0x34. era -O2 -G8.
 */
extern int D_8009D0E8;

int func_8005E518(void) {
    int r = 0;
    if (D_8009D0E8) {
        unsigned int v = func_8005E038() & 0x5000;
        r = v != 0;
    }
    return r;
}
