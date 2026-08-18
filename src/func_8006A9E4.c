/*
 * IN PROGRESS — Phase 5FO: func_8006A9E4 (main's boot-read callee, 215 words).
 * DO NOT INTEGRATE — checkpoint candidate, NOT matched.
 * Best state: draft d10 (session 2026-08-18, second round) — 216 words (+1),
 * frame EXACT (0x30; vars 8 + 5x$s + $ra + args 16), all zone/copy STRUCTURE
 * word-count-exact. The single structural extra is the zone-1 gate steal
 * (698D4-class dbr_sched divergence). Remaining byte deltas are register/
 * offset choices inside the copy loops and the zone-3/4 gate li extras.
 *
 * SEMANTICS (retail-decoded, proven against 0x5B1E4-0x5B540):
 *   RECT16 {0,0,0x3FF,0x1FF} local at sp+0x10; ClearImage(&rect,0,0,1)
 *   (func_80074F44; the h=0x1FF store lands in the jal delay slot).
 *   s3 = &D_800B0CD8 (data struct), s2 = D_800B0DD8 (mount base).
 *   FOUR table-driven read zones over the u16 boundary pairs at
 *   D_800930DC[0],[1],[4],[5] ({off,end}: len = end-off, src = base+off):
 *     retry func_8006E6A8(dst, buf, len) while == -1; then the GOTO-GATE
 *     poll: flag = 1; poll: if (flag == -1) goto restart;
 *     flag = func_8006E7E8();  (zone 3/4 booleanized via sltu: flag=(r!=0))
 *     if (flag != 0) goto poll.
 *     Zone 1 dst = D_800A8028; zones 2/3/4 dst = *(data+0x194) loaded
 *     INSIDE the retry loop (write the expression in the call argument).
 *     Zone 2 additionally calls func_800527C8() once (called-flag at the
 *     poll-loop head; flag=1 store sits in the jal delay slot).
 *   After zone 2: copy 0x10A50 bytes *(data+0x194) -> D_800E2858; alignment
 *   split on ((src|dst)&3): unaligned = 16-byte lwl/lwr + swl/swr blocks,
 *   aligned = lw/sw; do/while shape (retail has NO trip guard); end =
 *   src + 0x10A50 (lui hoisted into the beqz slot, ori on both paths).
 *   data[0x148] = &D_800E2858 (store in first 6E498 jal slot);
 *   data[0x140] = func_8006E498(&D_800E2858, 0x57D40D84) (store in second
 *   jal slot); data[0x144] = SECOND call's return (post-call sw).
 *   Zone 3 + func_80087090(D_800B0E6C, 1) (symbol form, not data+0x194).
 *   Zone 4, then copy 2: 0x1400 bytes *(data+0x194) -> *(data+0x130).
 *
 * PROVEN LEVERS (this session, rounds 1-2):
 *   - goto-gate form reproduces retail's dead-edge outer/retry/poll layout
 *   - inline *(data+0x194) call arg = per-iteration load; removed 6th $s
 *     (frame 56 -> 48 exact)
 *   - pins data->asm("$19"), base->asm("$18"), t->asm("$16") natural-fit
 *   - called->asm("$17") pin flips zone 2 to retail's called=$s1 /
 *     sentinel2=$s4 assignment (d8/d10)
 *   - __builtin_memcpy(dst, src, 16) on char* emits EXACTLY retail's
 *     lwl/lwr x4 + swl/swr x4 block shape (proven isolated AND in-function)
 *   - do/while copy = no trip guard (retail shape)
 *   NEGATIVE RESULTS (do not retry): packed struct -> byte-wise synthesis
 *   (+119 words); aligned(1) struct attr -> ignored (plain lw/sw);
 *   -fno-strength-reduce -> 208/221-word regressions; OVERLAPPING double
 *   pin on $17 (flag AND sentinel) COLLAPSES codegen to 40 words.
 *
 * REMAINING DELTAS (d10 -> retail):
 *   (a) zone-1 retry back-edge slot: retail NOP, ours steals the gate li -1
 *       (698D4-class). Sentinel VARIABLE in zones 1/3 fixes the steal and
 *       removes the extra li, but RIPPLES zone 2 (d9/d11 regression) —
 *       next: combine called pin + sentinel vars + zone-2 sentinel pin $20.
 *   (b) copy loops: ours strength-reduced (base+offset via separate
 *       induction regs, e.g. lw $v0,-8($a0)) vs retail clean base+offset
 *       (lw $v1,4($a2)); same word counts, different encodings. Retail's
 *       aligned loop batches 4 loads (lw v0/v1/a0/a1 consecutive) — our
 *       temp-batched attempt (d12) added load-delay nops instead (+9).
 *   (c) zone-3/4 gates: bare -1 materialized to $v0 (extra li, one balanced
 *       by retail's own sltu-slot copy). Sentinel-var + pin combo should
 *       close these with (a).
 *   (d) 0x10A50 lui/ori split: retail hoists the lui into the alignment
 *       beqz slot, ori executes on both paths; ours keeps them together.
 *
 * ROUND-3 DATAPOINTS (d13/d14/d15 — bounded gate-vs-copies matrix):
 *   d13 (function-scope sentinel vars + called pin + memcpy, default opts):
 *       225 words — sentinel vars ripple zone 2+ despite fixing zone 1/3.
 *   d14 (BLOCK-SCOPED sentinel pins: $17 in zone-1/3 blocks, $20 in zone-2/4
 *       blocks, disjoint scopes; called function-scope $17): compiles cleanly
 *       and makes ALL zone gates word-exact (zero mismatches through w069)
 *       but BLOATS the copy regions to 226 words — pinned sentinel liveness
 *       steals registers from the copy loops.
 *   d15 (d10 zones + __builtin_memcpy unaligned): 224 words — memcpy costs
 *       +8 vs d10's strength-reduced unaligned copy in this pressure context.
 *   CONCLUSION: d10<->d14 brackets the solution — a gates-vs-copies register-
 *   pressure trade-off. Next levers: narrow sentinel lifetimes further (kill
 *   before the copies), or pin the COPY cursors and free the gates, or
 *   accept the d10 residual and close the gates with 5FK-style barriers only
 *   where the word budget allows.
 * ROM: asm/disc1/5B1E4.s @ file 0x5B1E4, 215 words (0x35C), frame 0x30.
 * This file mirrors d10 (best checkpoint). Scratch: /tmp/5fo/ (session-local).
 */

typedef struct { short x, y, w, h; } RECT16;
typedef struct { unsigned int w; } __attribute__((aligned(1))) U32U;

extern unsigned int D_800B0CD8;
extern unsigned char *D_800B0DD8;
extern unsigned short D_800930DC[];
extern unsigned char D_800A8028[];
extern unsigned char D_800E2858[];
extern unsigned char *D_800B0E6C;

extern void func_80074F44(void *rect, int r, int g, int b);
extern int  func_8006E6A8(unsigned char *dst, unsigned char *buf, int len);
extern int  func_8006E7E8(void);
extern void func_800527C8(void);
extern int  func_8006E498(unsigned char *p, unsigned int code);
extern void func_80087090(unsigned char *p, int n);

void func_8006A9E4(void) {
    RECT16 rect;
    register unsigned char *data asm("$19") = (unsigned char *)&D_800B0CD8;
    register unsigned char *base asm("$18") = D_800B0DD8;
    register unsigned short *t asm("$16");
    unsigned char *buf, *dst;
    int flag;
    register int called asm("$17");

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x3FF;
    rect.h = 0x1FF;
    func_80074F44(&rect, 0, 0, 1);

z1_again:
    t = &D_800930DC[0];
    while (func_8006E6A8(base + t[0], D_800A8028, t[1] - t[0]) == -1)
        ;
    flag = 1;
z1_poll:
    if (flag == -1)
        goto z1_again;
    flag = func_8006E7E8();
    if (flag != 0)
        goto z1_poll;

    called = 0;

z2_again:
    t = &D_800930DC[1];
    while (func_8006E6A8(base + t[0], *(unsigned char **)(data + 0x194), t[1] - t[0]) == -1)
        ;
    flag = 1;
z2_poll:
    if (!called) {
        func_800527C8();
        called = 1;
    }
    if (flag == -1)
        goto z2_again;
    flag = func_8006E7E8();
    if (flag != 0)
        goto z2_poll;

    buf = *(unsigned char **)(data + 0x194);
    dst = D_800E2858;
    if (((unsigned int)buf | (unsigned int)dst) & 3) {
        unsigned char *end = buf + 0x10A50;
        do {
            ((U32U *)(dst + 0))->w  = ((U32U *)(buf + 0))->w;
            ((U32U *)(dst + 4))->w  = ((U32U *)(buf + 4))->w;
            ((U32U *)(dst + 8))->w  = ((U32U *)(buf + 8))->w;
            ((U32U *)(dst + 12))->w = ((U32U *)(buf + 12))->w;
            buf += 16;
            dst += 16;
        } while (buf != end);
    } else {
        unsigned char *end = buf + 0x10A50;
        do {
            *(int *)(dst + 0)  = *(int *)(buf + 0);
            *(int *)(dst + 4)  = *(int *)(buf + 4);
            *(int *)(dst + 8)  = *(int *)(buf + 8);
            *(int *)(dst + 12) = *(int *)(buf + 12);
            buf += 16;
            dst += 16;
        } while (buf != end);
    }

    *(unsigned char **)(data + 0x148) = D_800E2858;
    *(int *)(data + 0x140) = func_8006E498(D_800E2858, 0x57D40D84u);
    *(int *)(data + 0x144) = func_8006E498(*(unsigned char **)(data + 0x148), 0x57D41D84u);

z3_again:
    t = &D_800930DC[4];
    while (func_8006E6A8(base + t[0], *(unsigned char **)(data + 0x194), t[1] - t[0]) == -1)
        ;
    flag = 1;
z3_poll:
    if (flag == -1)
        goto z3_again;
    flag = (func_8006E7E8() != 0);
    if (flag != 0)
        goto z3_poll;

    func_80087090(D_800B0E6C, 1);

z4_again:
    t = &D_800930DC[5];
    while (func_8006E6A8(base + t[0], *(unsigned char **)(data + 0x194), t[1] - t[0]) == -1)
        ;
    flag = 1;
z4_poll:
    if (flag == -1)
        goto z4_again;
    flag = (func_8006E7E8() != 0);
    if (flag != 0)
        goto z4_poll;

    buf = *(unsigned char **)(data + 0x194);
    dst = *(unsigned char **)(data + 0x130);
    if (((unsigned int)buf | (unsigned int)dst) & 3) {
        unsigned char *end = buf + 0x1400;
        do {
            ((U32U *)(dst + 0))->w  = ((U32U *)(buf + 0))->w;
            ((U32U *)(dst + 4))->w  = ((U32U *)(buf + 4))->w;
            ((U32U *)(dst + 8))->w  = ((U32U *)(buf + 8))->w;
            ((U32U *)(dst + 12))->w = ((U32U *)(buf + 12))->w;
            buf += 16;
            dst += 16;
        } while (buf != end);
    } else {
        unsigned char *end = buf + 0x1400;
        do {
            *(int *)(dst + 0)  = *(int *)(buf + 0);
            *(int *)(dst + 4)  = *(int *)(buf + 4);
            *(int *)(dst + 8)  = *(int *)(buf + 8);
            *(int *)(dst + 12) = *(int *)(buf + 12);
            buf += 16;
            dst += 16;
        } while (buf != end);
    }
}
