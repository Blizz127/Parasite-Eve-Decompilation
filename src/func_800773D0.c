/* Phase 5FZ: matching C leaf.
 * VRAM 0x800773D0 / file 0x67BD0 / size 0x34 (13 words).
 *
 * Reset a countdown pair: the timer field D_80095888 is armed to the VSync clock
 * (func_80073A44(-1)) plus a 0xF0 grace period, and the paired counter
 * D_8009588C is cleared.  func_80077404 (still asm) is the poll side that
 * compares these against the same clock.
 *
 * Both globals are absolute here, so the default -O2 -G0 profile matches.
 */
extern int D_80095888;
extern int D_8009588C;

int func_80073A44(int a0);

void func_800773D0(void) {
    D_80095888 = func_80073A44(-1) + 0xF0;
    D_8009588C = 0;
}
