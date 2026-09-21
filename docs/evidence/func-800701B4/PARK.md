# `func_800701B4` — PARKED (callee-saved register ordering residual)

Status: **PARKED**, not a matching leaf. 9 of 74 words differ, all in the
function prologue; the loop body, loop tail, arena lookups, clear loop,
flag update and epilogue are **byte-identical** to retail.

## Function hood and retail span

- File span `[0x609B4,0x60ADC)` = `0x128` bytes = 74 words (left as `asm`).
- VRAM span `[0x800701B4,0x800702DC)`.
- Semantic role: E8-arena teardown sweep — loop ids `0xB..0x15`
  (`idx = i + 0xB`), `func_8006FC18(idx, 0, 1)`, and on a 0 return clear
  the record plus the `D_800E0EF0[0x6C..0x72]` / `D_800B0CD8` bit `0x10000`
  cleanup. Structurally the E8 twin of matched `func_800702DC`.

## Closest C shape (LINK residual 9 words)

```c
int func_800701B4(void) {
    int r;
    int i;
    int idx;
    unsigned char *q;

    r = 0;
    for (i = 0, idx = 0xB; i < 0xB; idx++, i++) {
        r = func_8006FC18(idx, 0, 1);
        if (r != 0)
            return r;
        if ((unsigned int)idx < 0x16) {
            if ((unsigned int)idx >= 0xB)
                q = D_800942E8 + (idx - 0xB) * 0x10C;
            else
                q = D_800942E4 + idx * 0xA0C;
            if (q[1] == 0x72) {
                unsigned int k;
                for (k = 0x6C; k < 0x73; k++)
                    D_800E0EF0[k] = 0;
                D_800B0CD8 &= 0xFFFEFFFF;
            }
            q[0] = 0; q[1] = 0xFF; q[2] = 0xFF; q[3] = 0xFF;
            *(int *)(q + 4) = 0;
            *(int *)(q + 8) = 0;
        }
    }
    return r;
}
```

Command:

```text
tools/analysis/era_leaf_match.sh <file> 0x800701B4 0x128 -O2 -G0
```

- Object (reloc-normalised) size `0x130` vs retail `0x128` (GNU as pad).
- Linked at `0x800701B4`: `word mismatches=9` (all prologue).

## Exact residual

Retail prologue order (callee-saved `sw` interleaved with the init of each
register):

```text
800701b8 sw   $s3,28(sp)   ; i = 0
800701bc move $s3,$zero
800701c0 sw   $s4,32(sp)   ; tbl = D_800E0EF0
800701c4 lui  $s4,0x800e
800701c8 addiu $s4,$s4,0xef0
800701cc sw   $s2,24(sp)   ; off (E4) = 0xB*0xA0C = 28292
800701d0 li   $s2,28292
800701d4 sw   $s1,20(sp)   ; off8 (E8) = 0
800701d8 move $s1,$zero
800701dc sw   $s0,16(sp)   ; idx = 0xB
800701e0 li   $s0,11
800701e4 sw   $ra,36(sp)
```

Compiled (`-O2 -G0`) order for the same source:

```text
800701b8 sw   $s3,28(sp)   ; i = 0
800701bc move $s3,$zero
800701c0 sw   $s0,16(sp)   ; idx = 0xB   <-- scheduled 2nd instead of last
800701c4 li   $s0,11
800701c8 sw   $s4,32(sp)   ; tbl
800701cc lui  $s4,0x800e
800701d0 addiu $s4,$s4,0xef0
800701d4 sw   $s2,24(sp)   ; off (E4)
800701d8 li   $s2,28292
800701dc sw   $s1,20(sp)   ; off8 (E8)
800701e0 move $s1,$zero
800701e4 sw   $ra,36(sp)
```

So the only difference is the position of the `idx` initialisation (and its
`sw`): GCC schedules `li $s0,11` immediately after the loop counter init,
while retail schedules it after the loop-invariant table-base and the two
giv initialisers. Everything downstream of `800701e8` is exact.

The loop is a `for (i = 0, idx = 0xB; i < 0xB; idx++, i++)` with `idx` a
basic induction variable (retail `li $s0,11` / `addiu $s0,$s0,1` in the
branch delay). The residual is therefore purely the preheader instruction
schedule / callee-saved allocation order, not the semantics.

## What was tried (all still 9 mismatches unless noted)

- `for (i = 0, idx = 0xB; i < 0xB; i++, idx++)` and the swapped comma order
  (swapped order fixes the *loop tail* to exact — preheader still 9).
- `idx = i + 0xB` recomputed per iteration (no IV reduction; 27).
- `idx` assigned in the loop body / nested-block declaration (27).
- `idx` as a declaration initializer, `register int idx`, declaration-order
  permutations, `unsigned int idx` (9–11).
- `i = 0;` / `idx = 0xB;` as separate statements, `for (idx = 0xB; …)`,
  `while`/`do` forms (9–12).
- `i <= 0xA` bound (9).
- E4-first branch polarity (14).
- Explicit `unsigned int *tbl = D_800E0EF0;` local (65 — introduces a 6th
  callee-saved register).
- Era rungs: `-O1 -G0`, `-O1 -G0 -fschedule-insns2`,
  `-O2 -G0 -fschedule-insns2`, `-fno-schedule-insns[2]`,
  `-fno-delayed-branch` — all worse; `-O2 -G0` is the closest.

## Next step to unblock

Find the source shape that makes cc1 2.7.2 emit the `idx` initialiser after
the loop-invariant/giv initialisers (or a scheduling flag that reproduces
retail's preheader order) without perturbing the already-exact loop tail.
The body can be reused verbatim from `func_800702DC` / this file once the
preheader order is solved.
