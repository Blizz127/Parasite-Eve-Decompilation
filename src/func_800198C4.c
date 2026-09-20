/*
 * decomp-continue-4 — func_800198C4, file 0xA0C4, size 0x40.
 * func_8002FAD8(D_8009D2F0, a0[0] as byte, a0[1] as int, a0[2] as int); return 1.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int *D_8009D2F0;
extern int func_8002FAD8(int *, int, int, int);
int func_800198C4(int **a0) {
    func_8002FAD8(D_8009D2F0, *(unsigned char *)a0[0], *(int *)a0[1], *(int *)a0[2]);
    return 1;
}
