/*
 * decomp-continue-4 — func_8001A43C, file 0xAC3C, size 0x38.
 * func_8005401C() result stored through the word at *a0; returns 1.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_8005401C(void);
int func_8001A43C(int **a0) {
    int v0 = func_8005401C();
    *a0[0] = v0;
    return 1;
}
