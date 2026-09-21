/*
 * decomp-continue-4 — func_80077CF4, file 0x684F4, size 0x3C.
 * Sign-absolute wrapper: func_80077D30((a0<0 ? -a0 : a0) & 0xFFF), negated
 * for the negative input.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_80077D30(int);
int func_80077CF4(int a0) {
    if (a0 < 0)
        return -func_80077D30((-a0) & 0xFFF);
    return func_80077D30(a0 & 0xFFF);
}
