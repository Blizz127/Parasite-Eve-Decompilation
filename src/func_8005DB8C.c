/* func_8005DB8C — VRAM 0x8005DB8C, size 0x20, file 0x4E38C-0x4E3AC.
 *
 * Record-base helper: takes the word at D_800A8038, adds the 512-byte-strided
 * argument (`a0 << 9`) and the address of D_800A8038 minus 0x10, and returns
 * the sum. era_o2_g0.
 *
 * The `x` temporary keeps `a0 << 9` ahead of `base - 4` in the emitted
 * schedule; folding it into the final expression lets the scheduler swap the
 * two, which no longer matches retail 0x00042240 / 0x2443FFF0.
 */
extern int D_800A8038;
int func_8005DB8C(int a0) {
    int *base = &D_800A8038;
    int x = a0 << 9;
    int *p = base - 4;

    return *base + (x + (int)p);
}
