/* func_8006DBE0 — VRAM 0x8006DBE0, size 0x38, file 0x5E3E0-0x5E418.
 *
 * Searches the two signed bytes at D_800B0CD8+0xDC (stride 2) for a0 and
 * returns the matching index, or -1 when neither entry matches.
 *
 * era_o2_g0. The address must be taken through the extern symbol, not the
 * literal 0x800B0CD8: a literal makes cc1 emit `ori` for the low half, while
 * `&D_800B0CD8` emits the retail `addiu` (0x24A50CD8).
 */
extern unsigned char D_800B0CD8;
int func_8006DBE0(int a0) {
    int i;
    unsigned char *p;

    i = 0;
    p = &D_800B0CD8;
    while (i < 2) {
        if (*(signed char *)(p + 0xDC) == a0)
            return i;
        p += 2;
        i++;
    }
    return -1;
}
