/*
 * PARKED — Phase 5FM: func_8001220C (main, Boot Rung 1 keystone).
 *
 * Status: ~180/187 words match by opcode/position under era -O2 -G0, but the
 * residual is pervasive — 150 word-level mismatches across all zones when
 * alignment is considered.  The root cause is a prologue save-batching and
 * global register-assignment divergence between era cc1 and retail ccpsx:
 *
 *   - Prologue save order/interleaving differs fundamentally (retail saves
 *     $ra/$s4/$s3/$s2/$s1 up front and $s0 in the first jal delay slot;
 *     cc1 saves in a different sequence and interleaves saves into jal
 *     delay slots differently from retail).
 *   - Register assignment skew: cc1 assigns state_val (0xA9400048) to $s2
 *     and flagbyte (data+0xF5) to $s4; retail has them in $s3 and $s2
 *     respectively.  This swap cascades through all operand fields.
 *   - The invariant bitmask 0x100000 (retail $s4) materializes at a
 *     different point in the instruction stream.
 *   - The 9-word scratchpad stack-handoff atom (switch $sp to PS1
 *     scratchpad, call overlay, restore) is proven byte-exact as a fenced
 *     inline-asm block with complete caller-saved clobbers.
 *
 * Phase 5FK-style hard-register locals and zero-code barriers cannot address
 * a global register-allocation + save-batching problem of this scope.  The
 * bounded V0-V6 barrier matrix was NOT executed because the V0 baseline
 * already shows 150 mismatches — a localized barrier cannot fix pervasive
 * register-assignment skew cascading through 187 words.
 *
 * The candidate IS semantically complete: all 20 callee declarations, the
 * scratchpad atom, the main mount/read/dispatch loop, the volume/media gate,
 * and the A8-code three-way state switch are correct.  It remains suitable
 * for a PC-port native-execution path even though the exact-matching
 * production build stays at 229 leaves.
 *
 * ROM: asm/disc1/2A0C.s, 0x2A0C–0x2CF8, 187 words (0x2EC).
 * Next: func_800124F8 at 0x2CF8.
 */

/* main draft 5: func_8001220C — era -O2 -G0, NO 3W flag.
 * ONE change from draft 4: bitmask initialization moved after the first
 * two calls (func_800725DC + func_8003E610), matching ROM's w014-w015
 * materialization point. Declaration stays function-scope for $s3 holding.
 * Predicted: li 0x100000 vacates 725DC's delay slot; sw $s0 fills it. */

extern void func_800725DC(void);
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A9E4(void);
extern void func_8006AD40(void);
extern void func_8006ECEC(void);
extern void func_8006F044(void);
extern void func_8006E834(void);
extern void func_80069B08(int);
extern void func_8003F3C4(void);
extern void func_801235DC(void);
extern void func_8019234C(void);
extern int  func_801909B4(void);
extern int  func_8006E9A0(int);
extern int  func_800698D4(void);
extern void func_80073A44(int);
extern void func_80074D28(int);

extern unsigned int D_800B0CD8;
extern unsigned int D_8009D280;
extern unsigned int D_8009D1C4;
extern unsigned int D_800A7918;

void func_8001220C(void) {
    unsigned int *data      = &D_800B0CD8;           /* $s0 */
    int            dispatch = 0;                     /* $s1 */
    unsigned char *flagbyte = (unsigned char *)data + 0xF5;  /* $s2 */
    unsigned int   state_val = 0xA9400048u;          /* $s3 */
    unsigned int   bitmask;                          /* $s4 — DECLARED, not init'd */
    unsigned int v;

    func_800725DC();
    func_8003E610();
    bitmask = 0x00100000u;                           /* initialized HERE (ROM w014-w015) */

    for (;;) {
        func_8006A5BC();

        while (func_800698D4() == 0) {
            func_80073A44(0);
        }

        func_8006A64C();
        func_8003E680();
        func_8006A9E4();
        D_8009D280 = state_val;

        while (1) {
            v = *data;
            if (v & bitmask) {
                func_80069B08(dispatch);
                *data &= 0xFFEFFFFFu;
            }

            func_8006AD40();
            v = D_8009D280;
            D_8009D1C4 = v;

            /* A8-code three-way dispatch */
            if (v == state_val) {
                func_8006E834();
                v = func_801909B4();
                func_8006E9A0(v);
                *data |= 0x3;
            } else if (state_val < v) {
                if (v == 0xAA108448u) {
                    if (*flagbyte & 0x2) {
                        func_8006F044();
                        func_801235DC();
                        D_8009D280 = 0xA80830C8u;
                        *data |= 0x1;
                    } else {
                        dispatch = 2;
                        *data |= bitmask;
                    }
                } else {
                    func_8003F3C4();
                }
            } else {
                if (v == 0xA8000048u) {
                    func_8006ECEC();

                    /* === FENCED INLINE ASM ===
                     * ROM 0x2B20..0x2B44, 10 words.
                     * Re-points $sp to PS1 scratchpad top 0x1F8003FC,
                     * calls overlay 8019234C on the foreign stack, restores.
                     * Fenced, documented exception (register-pinning precedent).
                     * EXACT ROM mnemonics. */
                    __asm__ volatile (
                        "lui    $a1, 0x1F80\n\t"
                        "ori    $a1, $a1, 0x3FC\n\t"
                        "addu   $t0, $a1, $zero\n\t"
                        "sw     $sp, 0($t0)\n\t"
                        "addiu  $t0, $t0, -4\n\t"
                        "addu   $sp, $t0, $zero\n\t"
                        "jal    func_8019234C\n\t"
                        "nop\n\t"
                        "addiu  $sp, $sp, 4\n\t"
                        "lw     $sp, 0($sp)"
                        :
                        :
                        : "$1", "$2", "$3", "$4", "$5", "$6", "$7",
                          "$8", "$9", "$10", "$11", "$12", "$13",
                          "$14", "$15", "$24", "$25", "$31",
                          "memory"
                    );
                    /* === END ATOM === */

                    *data |= 0x1;
                } else {
                    func_8003F3C4();
                }
            }

            /* 7-constant skip chain */
            v = D_8009D280;
            if (v == 0xA80651C8u) continue;
            if (v == 0xA8065248u) continue;
            if (v == 0xA80652C8u) continue;
            if (v == 0xA80660C8u) continue;
            if (v == 0xA8066148u) continue;
            if (v == 0xA80661C8u) continue;
            if (v == 0xA8066348u) continue;

            /* volume gate */
            v = D_800A7918;
            if (v < 0x258u) {
                if (!(*flagbyte & 0x1)) {
                    dispatch = 1;
                    *data |= bitmask;
                }
            } else {
                if (!(*flagbyte & 0x2)) {
                    dispatch = 2;
                    *data |= bitmask;
                }
            }

            if (*data & 0x100) {
                func_80073A44(0);
                func_80074D28(0);
                *data &= ~0x101u;
                break;
            }
        }
    }
}