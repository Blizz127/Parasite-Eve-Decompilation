/* VRAM 0x8005E850 / file 0x4F050 / size 0x34.
 * Offset both args by signed-byte globals, then forward. */
extern signed char D_800B0DB0, D_800B0DB1;
extern void func_8006A2E8(int a0, int a1);
void func_8005E850(int a0, int a1) {
    func_8006A2E8(D_800B0DB0 + a0, D_800B0DB1 + a1);
}
