/*
 * decomp-continue-4 — func_80080F64, file 0x71764, size 0x34.
 * When (a0 & 0xFF) == 2, registers func_80080F98 with arg -1 through
 * func_80081D74.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern void func_80081D74(void (*fn)(void), int);
extern void func_80080F98(void);
void func_80080F64(int a0) {
    if ((a0 & 0xFF) == 2)
        func_80081D74(func_80080F98, -1);
}
