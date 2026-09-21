/* func_8006EC84 — VRAM 0x8006EC84, size 0x68, file 0x5F484-0x5F4EC.
 *
 * Iterates `count` slot-offset words in the bank at `base` and dispatches
 * each resolved address through func_800718D0. The index is read back to the
 * retail `sll $v0,$s0,16 / sra $v0,14` scaling only when it is narrowed to
 * 16 bits at the access (`(short)i`); a plain int index makes cc1 strength-
 * reduce the whole access into an advancing pointer instead.
 *
 * era -O2 -G0.
 */
extern void func_800718D0(int);
void func_8006EC84(int base, int count) {
    int i;
    for (i = 0; i < count; i++)
        func_800718D0(base + ((int *)base)[(short)i]);
}
