/* VRAM 0x800E0024 / file 0xD0824 / size 0x3C. */
int func_800E0024(int arg0, int arg1) {
    int d = arg0 - arg1;
    int v = d;
    if ((short)d < 0) {
        v = -d;
    }
    if ((short)v >= 0x801) {
        v = 0x1000 - v;
    }
    return (short)v;
}
