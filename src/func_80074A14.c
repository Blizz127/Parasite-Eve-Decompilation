/* Phase 5EF: exchange D_800956EC and return the prior value.
 * Reader provenance: low byte is stored, masked with 0xFF, and state-tested. */
extern unsigned int D_800956EC;

unsigned int func_80074A14(unsigned int value) {
    unsigned int old = D_800956EC;
    D_800956EC = value;
    return old;
}
