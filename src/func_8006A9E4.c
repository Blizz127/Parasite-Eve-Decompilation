/*
 * IN PROGRESS — Phase 5FO: func_8006A9E4 (main's boot-read callee, 215 words).
 * DO NOT INTEGRATE — checkpoint candidate, NOT matched.
 * Best state: draft d6 (session 2026-08-18) — frame EXACT (0x30; vars 8 +
 * 5x$s + $ra + args 16), control structure proven, 218 words (+3), ~178
 * positional mismatches dominated by relocation placeholders and three
 * identified microstructure deltas below.
 *
 * SEMANTICS (retail-decoded, proven against 0x5B1E4-0x5B540):
 *   RECT16 {0,0,0x3FF,0x1FF} local at sp+0x10; ClearImage(&rect,0,0,1)
 *   (func_80074F44; the h=0x1FF store lands in the jal delay slot).
 *   s3 = &D_800B0CD8 (data struct), s2 = D_800B0DD8 (mount base).
 *   FOUR table-driven read zones over the u16 boundary pairs at
 *   D_800930DC[0],[1],[4],[5] ({off,end}: len = end-off, src = base+off):
 *     each zone:  retry issue func_8006E6A8(dst, buf, len) while == -1;
 *     then the GOTO-GATE poll structure —
 *         flag = 1;
 *     poll: if (flag == -1) goto zone_restart;   (dead-ish gate; live only
 *                                                if poll returns -1)
 *           flag = func_8006E7E8();              (zone 3/4: booleanized
 *                                                via sltu: flag = (r != 0))
 *           if (flag != 0) goto poll;
 *     Zone 1 dst = D_800A8028; zones 2/3/4 dst = *(data+0x194) loaded
 *     INSIDE the retry loop (per-iteration lw $a1,0x194($s3) — write the
 *     expression in the call argument, NOT a hoisted local).
 *     Zone 2 additionally calls func_800527C8() once (called-flag at the
 *     poll-loop head; flag=1 store sits in the jal delay slot).
 *   After zone 2: copy 0x10A50 bytes *(data+0x194) -> D_800E2858; alignment
 *   split on ((src|dst)&3): unaligned = 16-byte lwl/lwr + swl/swr blocks,
 *   aligned = lw/sw; do/while shape (retail has NO trip guard); loop test
 *   bne src,end with dst+=16 in the slot; end = src + 0x10A50 (lui hoisted
 *   into the beqz slot, ori executed on both paths).
 *   data[0x148] = &D_800E2858 (store in the first 6E498 jal slot);
 *   data[0x140] = func_8006E498(&D_800E2858, 0x57D40D84) (store in the
 *   second jal slot); data[0x144] = SECOND call's return (post-call sw).
 *   Zone 3 + func_80087090(D_800B0E6C, 1) (symbol form, not data+0x194).
 *   Zone 4, then copy 2: 0x1400 bytes *(data+0x194) -> *(data+0x130)
 *   (same alignment split; end = src + 0x1400 via single addiu in the
 *   beqz delay slot).
 *
 * PROVEN LEVERS (this session):
 *   - goto-gate form reproduces retail's dead-edge outer/retry/poll layout
 *   - inline *(data+0x194) in the call arg = per-iteration load, removes
 *     the 6th $s register (frame 56 -> 48 exact)
 *   - pins: data->asm("$19"), base->asm("$18"), t->asm("$16") natural-fit
 *   - OVERLAPPING double pin (flag AND sentinel both asm("$17")) BREAKS
 *     codegen (d7 collapsed to 40 words) — do not retry that shape
 *
 * REMAINING DELTAS (d6 -> retail):
 *   (a) retry back-edge slot: retail NOP, ours steals the gate's li -1
 *       into the slot (698D4-class dbr_sched decision; retail gate keeps
 *       -1 in $s1, ours materializes into $v0) — register-home + steal
 *   (b) called/sentinel2 register swap (ours called->$s4, retail $s1 with
 *       sentinel2->$s4)
 *   (c) copy loops: ours strength-reduced (negative-offset pointer form,
 *       single-word-per-iter reshuffle) vs retail offset-addressed 4x4
 *       batched blocks; unaligned branch needs the packed-type lever to
 *       emit lwl/lwr (untested at d6 — d7 died before proving it)
 *   (d) 0x10A50 lui/ori split scheduling into the beqz slot
 * ROM: asm/disc1/5B1E4.s @ file 0x5B1E4, 215 words (0x35C), frame 0x30.
 * Scratch: /tmp/5fo/d6.c (session-local) — this file mirrors d6.
 */

typedef struct { short x, y, w, h; } RECT16;

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
    unsigned char *buf, *dst, *end;
    int flag;
    int called;

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
    end = buf + 0x10A50;
    if (((unsigned int)buf | (unsigned int)dst) & 3) {
        while (buf != end) {
            *(unsigned int *)(dst + 0)  = *(unsigned int *)(buf + 0);
            *(unsigned int *)(dst + 4)  = *(unsigned int *)(buf + 4);
            *(unsigned int *)(dst + 8)  = *(unsigned int *)(buf + 8);
            *(unsigned int *)(dst + 12) = *(unsigned int *)(buf + 12);
            buf += 16;
            dst += 16;
        }
    } else {
        while (buf != end) {
            *(int *)(dst + 0)  = *(int *)(buf + 0);
            *(int *)(dst + 4)  = *(int *)(buf + 4);
            *(int *)(dst + 8)  = *(int *)(buf + 8);
            *(int *)(dst + 12) = *(int *)(buf + 12);
            buf += 16;
            dst += 16;
        }
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
    end = buf + 0x1400;
    if (((unsigned int)buf | (unsigned int)dst) & 3) {
        while (buf != end) {
            *(unsigned int *)(dst + 0)  = *(unsigned int *)(buf + 0);
            *(unsigned int *)(dst + 4)  = *(unsigned int *)(buf + 4);
            *(unsigned int *)(dst + 8)  = *(unsigned int *)(buf + 8);
            *(unsigned int *)(dst + 12) = *(unsigned int *)(buf + 12);
            buf += 16;
            dst += 16;
        }
    } else {
        while (buf != end) {
            *(int *)(dst + 0)  = *(int *)(buf + 0);
            *(int *)(dst + 4)  = *(int *)(buf + 4);
            *(int *)(dst + 8)  = *(int *)(buf + 8);
            *(int *)(dst + 12) = *(int *)(buf + 12);
            buf += 16;
            dst += 16;
        }
    }
}
