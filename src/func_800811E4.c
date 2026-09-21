/*
 * func_800811E4 — CD read poll (VRAM 0x800811E4, 28 words, 0x70).
 * ROM: asm/disc1/7155C.s (the pre-carve unit that held this function), era
 * gcc-2.7.2-psx -O2 -G0.
 *
 * Reads the current VSync counter (func_80073A44 = VSync(-1)), then:
 *   - if the read-issue timestamp (D_8009B6C4) is more than 1200 vsyncs
 *     behind it, call func_80081268 (timeout abort) and return -1;
 *   - otherwise return the pending-byte count (D_8009B6B4), 0 = complete.
 * func_8007F608(scratch) (PROBABLE DsDataSync) result is discarded.
 *
 * The two state words are adjacent, and retail keeps ONE base address in
 * $a0 across both loads (la $4,D_8009B6C4 / lw $3,0($4) / lw $16,-16($4))
 * rather than materializing each symbol separately. cc1 folds a bare
 * `sym[0]` / `sym[-4]` pair into two independent lui/lw addresses, so the
 * struct fields below are reached through a single `&issue` pointer and the
 * zero-code asm barrier stops copy-propagation from folding that pointer
 * back into the loads. Barrier emits no instructions.
 *
 * The frame teardown lands in the `jr $31` delay slot in retail (ASPSX's
 * reorder scheduler); cc1 leaves the stack adjust before the jump whenever
 * callee-saved registers are restored, so the build selects the
 * MASPSX_FILL_EPILOGUE_DELAY_SLOT profile for this leaf.
 */
extern int func_80073A44(int mode);          /* VSync */
extern void func_80081268(void);             /* CD read timeout abort */
extern void func_8007F608(unsigned char *p); /* PROBABLE DsDataSync */

typedef struct {
    int pending; /* 0x8009B6B4: pending bytes / sectors remaining */
    int unk08;   /* 0x8009B6B8 */
    int unk0C;   /* 0x8009B6BC */
    int unk10;   /* 0x8009B6C0 */
    int issue;   /* 0x8009B6C4: vsync timestamp of the last issue */
} CdState;

extern CdState D_8009B6B4;

int func_800811E4(unsigned char *p) {
    int vsync = func_80073A44(-1);
    int *base = &D_8009B6B4.issue;
    int status;

    asm volatile("" : "=r"(base) : "0"(base)); /* zero-code: keep the base in $a0 */
    if (base[0] + 1200 < vsync) {
        func_80081268();
        status = -1;
    } else {
        status = base[-4];
    }
    func_8007F608(p);
    return status;
}
