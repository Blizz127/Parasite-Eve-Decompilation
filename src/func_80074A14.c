/* Phase 5EF: exchange D_800956EC and return the prior value.
 * Type provenance: accepted int getter func_80074A28. */
extern int D_800956EC;

int func_80074A14(int value) {
    int old = D_800956EC;
    D_800956EC = value;
    return old;
}
