/*
 * decomp-continue-4 — func_800773D0, file 0x67BD0, size 0x34.
 *
 * Latches a VSync-based timestamp pair: D_80095888 = VSync(-1) + 0xF0 and
 * D_8009588C = 0.  The VSync call (func_80073A44) materializes -1 in its jal
 * delay slot; both globals are absolute-symbol stores (lui $at / sw).
 *
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_80073A44(int);
extern int D_80095888;
extern int D_8009588C;

void func_800773D0(void) {
    D_80095888 = func_80073A44(-1) + 0xF0;
    D_8009588C = 0;
}
