/* Phase 5EF: set opaque word D_8009B554 to one.
 * Readers are branch-only; refine if a narrowing reader is found. */
extern unsigned int D_8009B554;

void func_80080930(void) {
    D_8009B554 = 1;
}
