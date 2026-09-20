/*
 * func_800556E8 — bounds-checked signed halfword table getter (retail 0x800556E8).
 *
 * VRAM 0x800556E8 / file 0x45EE8 / size 0x3C (15 words), in 44AA0.s.
 *
 * Returns D_800A1D9C[a0] when 0 <= a0 < D_8009D040 (gp+0x2D0), else 0.
 *
 * era -O2 -G8 + MASPSX_PASSTHROUGH_SYMBOL_LOAD=1. `-G8` is required for
 * the gp count load (`lw $v0,0x2D0($gp)`); the passthrough knob folds the
 * table's %lo into the indexed `lh`
 * (`lui $at,%hi; addu $at,$at,$v0; lh $v0,%lo($at)`). Either alone leaves
 * 7-10 differing words.
 */
extern int D_8009D040;
extern short D_800A1D9C[];

int func_800556E8(int a0) {
    if (a0 < 0) {
        return 0;
    }
    if (a0 >= D_8009D040) {
        return 0;
    }
    return D_800A1D9C[a0];
}
