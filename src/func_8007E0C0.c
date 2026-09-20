/*
 * decomp-continue-4 — func_8007E0C0, file 0x6E8C0, size 0x38.
 * Critical section around func_8007E1F4(1, D_800A34B0); returns 1.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern void func_80072714(void);
extern int func_8007E1F4(int, char *);
extern void func_80072724(void);
extern char D_800A34B0[];
int func_8007E0C0(void) {
    func_80072714();
    func_8007E1F4(1, D_800A34B0);
    func_80072724();
    return 1;
}
