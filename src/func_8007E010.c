/*
 * decomp-continue-4 — func_8007E010, file 0x6E810, size 0x38.
 * Three-call void sequence then D_8009B4AC = 0.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern void func_8007E218(void);
extern void func_8007E1C4(void);
extern void func_8007E0C0(void);
extern int D_8009B4AC;
void func_8007E010(void) {
    func_8007E218();
    func_8007E1C4();
    func_8007E0C0();
    D_8009B4AC = 0;
}
