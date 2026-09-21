/*
 * decomp-continue-4 — func_8001A474, file 0xAC74, size 0x38.
 * func_80052F70() result stored through the word at *a0; returns 1.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_80052F70(void);
int func_8001A474(int **a0) {
    int v0 = func_80052F70();
    *a0[0] = v0;
    return 1;
}
