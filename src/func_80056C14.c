/*
 * func_80056C14 — 32-byte-stride halfword table getter (retail 0x80056C14).
 *
 * VRAM 0x80056C14 / file 0x47414 / size 0x2C (11 words), in 4680C.s.
 *
 * Returns the halfword at D_800A1E6E + (arg0 << 5) when arg0 < 3, else 0.
 *
 * era -O2 -G0 + MASPSX_PASSTHROUGH_SYMBOL_LOAD=1. The passthrough knob
 * is what folds the low half of D_800A1E6E into the `lhu` immediate
 * (`lui $at,%hi; addu $at,$at,$v0; lhu $v0,%lo($at)`). Without it cc1
 * emits the extra `addiu $at,$at,%lo` word and the branch offsets shift.
 */
extern unsigned short D_800A1E6E[];

int func_80056C14(unsigned int arg0) {
    return arg0 < 3 ? D_800A1E6E[arg0 << 4] : 0;
}
