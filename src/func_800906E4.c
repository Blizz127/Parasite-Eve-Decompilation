/*
 * func_800906E4 — clear a status bit, look up a byte, set flag bits, store.
 * VRAM 0x800906E4 / file 0x80EE4 / size 0x38 (14 words).
 * era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (three-word indexed load).
 */
extern unsigned char D_800B290C[];
void func_800906E4(unsigned char *a0) {
    unsigned int v = *(unsigned int *)(a0 + 0x38);
    unsigned char byte;
    unsigned int f;
    v &= ~0x200u;
    *(unsigned int *)(a0 + 0x38) = v;
    byte = D_800B290C[*(unsigned short *)(a0 + 0x5A) << 6];
    f = *(unsigned int *)(a0 + 0xF4) | 0x4400;
    *(unsigned int *)(a0 + 0xF4) = f;
    *(unsigned short *)(a0 + 0x116) = byte;
}
