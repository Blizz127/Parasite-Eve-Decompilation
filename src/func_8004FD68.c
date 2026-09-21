/*
 * decomp-continue-4 — func_8004FD68, file span in 3F17C/401A0.
 * func_80052E30(1) then func_800638D8(a0, func_80050BE8).
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern void func_80052E30(int);
extern int func_800638D8(int, void (*fn)(void));
extern void func_80050BE8(void);
void func_8004FD68(int a0) {
    func_80052E30(1);
    func_800638D8(a0, func_80050BE8);
}
