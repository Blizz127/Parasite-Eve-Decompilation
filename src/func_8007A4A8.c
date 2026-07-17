/* Phase 5EF: int state exchange; readers test D_8009AFB4 for zero. */
extern int D_8009AFB4;

int func_8007A4A8(int value) {
    int old = D_8009AFB4;
    D_8009AFB4 = value;
    return old;
}
