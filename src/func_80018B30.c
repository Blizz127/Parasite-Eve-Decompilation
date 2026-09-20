/*
 * decomp-continue-4 — func_80018B30, file 0x9330, size 0x38.
 *
 * Three-reader call wrapper, all short: a0[0], a0[1], a0[2] loaded as lh and
 * forwarded to func_800679C4; returns 1.
 *
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_800679C4(int, int, int);

int func_80018B30(int *a0) {
    func_800679C4(*(short *)a0[0], *(short *)a0[1], *(short *)a0[2]);
    return 1;
}
