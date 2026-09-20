/*
 * decomp-continue-4 — func_800193D8, file 0x9BD8, size 0x38.
 *
 * Three-int reader-call wrapper: a0[0], a0[1], a0[2] dereferenced as int and
 * forwarded to func_80065AD4; returns 1.
 *
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_80065AD4(int, int, int);

int func_800193D8(int *a0) {
    func_80065AD4(*(int *)a0[0], *(int *)a0[1], *(int *)a0[2]);
    return 1;
}
