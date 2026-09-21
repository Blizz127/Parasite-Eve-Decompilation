/* VRAM 0x80051684 / file 0x41E84 / size 0x30.
 * Two-level guarded store: value<<16 into +8 of the object at
 * (*D_8009D254)->[0]. */
extern unsigned char *D_8009D254;

void func_80051684(int value) {
    unsigned char *a = D_8009D254;
    if (a != 0) {
        unsigned char *b = *(unsigned char **)a;
        if (b != 0)
            *(int *)(b + 8) = value << 16;
    }
}
