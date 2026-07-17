/* Phase 5EF: return and clear the D_8009B78C int state flag.
 * Type provenance: YAML-live writer stores state value one. */
extern int D_8009B78C;

int func_80082CDC(void) {
    int old = D_8009B78C;
    D_8009B78C = 0;
    return old;
}
