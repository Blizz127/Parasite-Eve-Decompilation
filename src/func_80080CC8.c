/* Phase 5EF: signed state/count exchange.
 * Type provenance: YAML-live readers use blez and slti on D_8009AFC0. */
extern int D_8009AFC0;

int func_80080CC8(int value) {
    int old = D_8009AFC0;
    D_8009AFC0 = value;
    return old;
}
