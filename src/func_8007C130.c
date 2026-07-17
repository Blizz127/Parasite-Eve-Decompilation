/* Phase 5EF: write-only opaque 32-bit setter.
 * No YAML-live reader narrows signedness, pointer-ness, or semantics. */
extern unsigned int D_8009B260;

void func_8007C130(unsigned int value) {
    D_8009B260 = value;
}
