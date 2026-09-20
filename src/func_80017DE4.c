/*
 * decomp-continue-4 — func_80017DE4, file 0x85E4, size 0x3C.
 * func_80037864() narrowed to signed char and stored through *a0[0]; return 1.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_80037864(void);
int func_80017DE4(int **a0) {
    signed char v = (signed char)func_80037864();
    *a0[0] = v;
    return 1;
}
