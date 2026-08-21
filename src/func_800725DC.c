/*
 * PARKED — Phase 5FN: func_800725DC (once-init callback runner, main's first
 * callee) and its twin func_8007264C.  PER-TU TOOLCHAIN SKEW (three coupled
 * mechanisms; see docs/ai_context/parked_blockers.json, per-tu-725dc).
 *
 * WHAT IT DOES (port-relevant): one-shot runner.  If D_80094538 (init flag)
 * is clear, set it to 1 and walk the function-pointer table at jtbl_80010000
 * (= VRAM 0x80010000, the load base of the main text), calling each entry;
 * the baked count is ZERO (loop body emitted but never executed).  The twin
 * func_8007264C runs the same loop when the flag is ALREADY set.  Both sit
 * immediately before the A0/B0 BIOS thunk stubs (func_800726B4 = A(0x39)).
 * ROM: asm/disc1/5F3E4.s @ file 0x62DDC, 28 words (0x70).
 *
 * BANKED LEVERS (durable, probe-verified in session 2026-08-18):
 *   1. TWO-WORD ZERO COUNT: retail materializes the loop count 0 as
 *      lui $s1,0x0000 / addiu $s1,$s1,0x0000 — NOT reachable from any C zero
 *      spelling (all fold to `move`; probed 10 forms).  SOLVED: the address
 *      expression `(int)jtbl_80010000 - 0x80010000` compiles to
 *      `la $r,SYM+0x7FFF0000` → R_MIPS_HI16/LO16 with addend, resolving to
 *      zero words 0000/0000 at link.  The loop SURVIVES -O2 with this form
 *      (a plain `int i = 0` bound is folded away with the whole loop body).
 *   2. NO ARGS AREA despite a call: retail frame is 0x10 (three saves + pad,
 *      no 16-byte o32 outgoing-args area) although the loop body calls via
 *      jalr.  Era cc1 reserves args=16 for every C call form probed (direct,
 *      indexed ptr, pinned ptr).  SOLVED: an inline-asm jalr call (with the
 *      counter decrement in the delay slot, tied "=r"/"0" operand) is not a
 *      CALL insn → frame = saves only.  Probed: .frame args=0.
 *   3. Loop shape proven under era -O2 -G0: lw/addiu/jalr/decrement-in-slot/
 *      bnez/nop + beqz-guard-with-nop all match retail word-for-word once
 *      registers are pinned ($s0=p cursor, $s1=count, $t0=$8 callback ptr).
 *
 * RESIDUAL — PER-TU TOOLCHAIN SKEW, three mechanisms, flag-invariant:
 *   (a) PROLOGUE SAVE ORDER: retail saves $s0@4, $s1@8, $ra@C — ascending
 *       contiguous.  Era cc1 emits descending ($ra first, offsets top-down)
 *       under -O2, -O1, -fno-schedule-insns, -fschedule-insns2, -G8 alike.
 *       PE1's OTHER prologues are descending or slot-interleaved (6E834
 *       matched era's descending; 3F3C4/69B08/6A5BC descending/interleaved);
 *       this unit is the outlier.  The coupled consequence: retail's bnez
 *       delay slot holds the store constant (ori $t0,$zero,1); ours holds
 *       the last save that reorg pulled into the slot.
 *   (b) li→ori EXPANSION: retail's slot constant is ORI $t0,$zero,1
 *       (0x34080001); the era pipeline's --dont-expand-li + GNU as path
 *       yields addiu.  ASPSX-default li→ori would match — per-TU assembler
 *       setting, third per-TU datapoint (after 6E834-vs-698D4 range-test
 *       folding).
 *   (c) FRAME MODEL: the no-args-area frame (mechanism 2) implies this unit
 *       was not compiled with the same ccpsx frame model as the matched
 *       units (197D0/6A5BC/6E834 all reserve the 16-byte area).
 * CONCLUSION: the SDK-runtime unit at 0x800725xx was built with a different
 * ccpsx/aspsx configuration than every era-matched unit so far.  Same class
 * as the cc1_investigation.md architectural skews; not source-expressible.
 * Candidate below is semantically complete (28 words; loop body exact under
 * era -O2 -G0) and suitable for a native-execution path.
 */

extern int D_80094538;                      /* init-once flag */
extern void (*jtbl_80010000[])(void);       /* callback table @ load base */

void func_800725DC(void) {
    if (!D_80094538) {
        register void (**p)(void) asm("$16");   /* retail $s0 cursor */
        register int i asm("$17");              /* retail $s1 count */
        D_80094538 = 1;
        p = jtbl_80010000;
        i = (int)jtbl_80010000 - 0x80010000;    /* link-time zero; banked lever 1 */
        while (i != 0) {
            register void (*f)(void) asm("$8") = *p;   /* retail $t0 */
            p = p + 1;
            /* asm call: keeps the frame at saves-only (banked lever 2) */
            __asm__ volatile ("jalr %1\n\taddiu %0,%0,-1"
                : "=r"(i) : "r"(f), "0"(i)
                : "$2","$3","$4","$5","$6","$7","$9","$10","$11","$12",
                  "$13","$14","$15","$24","$25","$31","memory");
        }
    }
}
