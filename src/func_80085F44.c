/* Change-guarded setter: read the current value, overwrite only on a change,
 * and return the previous value. VRAM 0x80085F44 / file 0x76744 / size 0x24. */
extern int D_8009B434;

int func_80085F44(int a0) {
    int v0 = D_8009B434;
    if (a0 != v0)
        D_8009B434 = a0;
    return v0;
}
