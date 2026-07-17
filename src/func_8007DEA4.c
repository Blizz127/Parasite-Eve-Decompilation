/* Phase 5EF: opaque 32-bit setter.
 * Readers are branch-only; refine if a narrowing reader is found. */
extern unsigned int D_8009B4AC;

void func_8007DEA4(unsigned int value) {
    D_8009B4AC = value;
}
