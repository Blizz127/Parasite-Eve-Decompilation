/*
 * func_80077D30 — quarter-wave sine table lookup returning a signed sample.
 * VRAM 0x80077D30 / file 0x68530 / size 0x90 (36 words).
 *
 * a0 is folded into the first quadrant over the two halfword tables
 * D_8009589C / D_8009489C; the far quadrants return the negated value.
 * era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (three-word indexed load).
 */
extern short D_8009489C[];
extern short D_8009589C[];

int func_80077D30(int a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401)
            return D_8009589C[a0];
        return D_8009589C[0x800 - a0];
    }
    if (a0 < 0xC01)
        return -D_8009489C[a0];
    return -D_8009589C[0x1000 - a0];
}
