/* VRAM 0x80056C14 / file 0x47414 / size 0x2C.
 * Return the halfword at D_800A1E6E + kind * 0x20 for kind < 3, else 0.
 * era -O2 -G0 + MASPSX_PASSTHROUGH_SYMBOL_LOAD=1 (the indexed symbol load
 * must expand as lui $at / addu $at,$at,$v0 / lhu $v0,lo($at), not the
 * 4-word lui/addiu/addu/lhu form). */
extern unsigned short D_800A1E6E[];

unsigned int func_80056C14(unsigned int kind) {
    return kind < 3 ? D_800A1E6E[kind << 4] : 0;
}
