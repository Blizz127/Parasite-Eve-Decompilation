/*
 * decomp-continue-4 — func_80015AB8, file 0x62B8, size 0x38.
 *
 * Three-reader call wrapper: a0[0] dereferenced as int, a0[1]/a0[2] as short,
 * forwarded to func_800677A0; returns 1.
 *
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_800677A0(int, int, int);

int func_80015AB8(int *a0) {
    func_800677A0(*(int *)a0[0], *(short *)a0[1], *(short *)a0[2]);
    return 1;
}
