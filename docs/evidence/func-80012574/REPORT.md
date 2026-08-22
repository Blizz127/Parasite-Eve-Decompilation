# func_80012574 — pointer-table relocation

## Target

VRAM `0x80012574..0x800125E0` (exclusive), file `0x2D74`, size `0x6C`
(27 words), from `asm/disc1/2D74.s`.

The leaf stores its input pointer to `D_8009CE04` (`gp + 0x94`). If the first
word is not bit-31 tagged, it adds the record base to word zero and to the
`+0x08` field of each `record[1]` entries. There are no calls or saved
registers; retail applies an 8-byte stack adjustment around both exits.

## First bounded attempt — non-match

Era `-O2 -G8` retained the GP-relative `D_8009CE04` accesses and emitted the
pointer-field loop in retail order, but produced `0x60` bytes (24 words), not
the required `0x6C` bytes. `scripts/build_us.sh` stopped at its trim guard:

```
ERROR: target size 0x6C > current 0x60
```

The first differing word is the unsigned tag test: retail uses `lui $v0,
0x8000; sltu $v0,$v0,$a2`, while era folds it to `bgez $v0`. Retail also has
the unconditional `addiu $sp,-8` / `addiu $sp,+8` pair that the frameless
candidate omits. Per the one-attempt cycle stop condition, no further C
phrasing or flag change was tried, and no verifier run or commit is claimed.

## Parked

Three bounded attempts were performed.  The final `0x6C` candidate matched
the unsigned tag test, empty 8-byte frame, GP access, and loop cursor form,
but remained non-exact by four bytes in the count/sum setup: retail allocates
that sequence as `$v1`/`$v0`, while the candidate uses `$v0`/`$a2`.

The leaf is parked: an exact match was not achieved, the candidate source and
split registration were restored to the exact baseline, and no commit was
made.  A further attempt requires a new explicit decision.
