/* Phase 5EF: int state exchange; readers test D_8009AFB8 for zero. */
extern int D_8009AFB8;

int func_8007A4BC(int value) {
    int old = D_8009AFB8;
    D_8009AFB8 = value;
    return old;
}
