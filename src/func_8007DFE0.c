/*
 * decomp-continue-4 — func_8007DFE0, file 0x6E7E0, size 0x30.
 *
 * Three-call forwarder: func_8007E1B4(), then func_80073C74(0) (the 0 is
 * materialized in the jal delay slot), then func_8007E204(); the retail then
 * loads $ra, sets $v0 = 1, and returns.
 *
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_8007E1B4(void);
extern int func_80073C74(int);
extern int func_8007E204(void);

int func_8007DFE0(void) {
    func_8007E1B4();
    func_80073C74(0);
    func_8007E204();
    return 1;
}
