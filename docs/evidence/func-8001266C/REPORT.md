# func_8001266C — boot work-table link init

## Target

VRAM `0x8001266C..0x80012700` (exclusive), file `0x2E6C`, size `0x94`
(37 words), from `asm/disc1/2F00.s` (resume span after the 125E0 carve).
Frameless leaf, no calls. Companion to `func_800124F8`: fills 0x47 row-link
words (stride 0x2C, the `D_8009D334` view of the `D_8009D310` rows) with the
address of each next row start (`base + 0x2C + 44*i`), nulls terminator
word `D_8009DF68` (= table end, absolute 2-word form), clears the
0x40-word array `D_800B6A80`, zeroes gp state word `D_8009D300`, and seeds
row-cursor `D_8009CDFC` with the table base. Counter is `unsigned short`
(0xFFFF masks at every use; sltiu bounds 0x47 / 0x40).

## First attempt — size gate failure

One minimal C candidate on era `-O2 -G8` +
`MASPSX_THREE_WORD_SYMBOL_STORE=1` (wired as `[0x2E6C, c]` with resume
`2F00.s`) failed the build's trim guard:

```
ERROR: build/src/func_8001266C.c.o .text: bytes beyond 0x94 are not all zero
```

The candidate emitted ~0x9C–0xA0 bytes (39–40 words) against the required
37. First differing word is offset 0x04. Exact divergences observed in the
object dump:

1. **Missing `&` on the RHS (semantic misread):** the candidate wrote
   `D_8009D310[i*11 + 11]` as the stored value, producing a memory load
   (`lw $v1,0($v1)`). Retail stores pure address arithmetic
   (`addu $v1,$v0,$a1`; no load). The stored value is the *address* of the
   next row start.
2. **Hoisted table base:** typing the table as a flat `unsigned int[]` made
   cc1 materialize `&D_8009D334` once into `$a2` and store register-
   relative (`addu $v0,$v0,$a2; sw $v1,0($v0)`). Retail re-emits the
   indexed-symbol store inside the loop every iteration
   (`lui $at,%hi(D_8009D334) / addu $at,$at,$v0 / sw $v1,%lo(...)($at)`),
   so the THREE_WORD gate never had a compound line to act on. The 5FD
   aggregate-element-type lever (real 44-byte element struct instead of
   flat words) is the documented countermeasure, but per the one-attempt
   rule it was NOT tried.
3. **Counter increment placement:** candidate hoists `addiu $a0,$a0,1` to
   the loop head; retail increments after the store inside the body.

## Parked

The leaf is parked after one attempt per the session's stop rule. The
candidate source (`src/func_8001266C.c`) and this report are left for
review; the split/build registration (`configs/USA/disc1.yaml`,
`scripts/build_us.sh`, `scripts/verify_us.sh`) was restored to the exact
276-leaf baseline (HEAD 669f39e) and `scripts/split_us.sh` re-run (stale
generated `asm/disc1/2F00.s` removed; `2E6C.s` regenerated). No commit was
made. A further attempt requires explicit authorization; the most promising
single change is the aggregate-element-type lever (item 2) combined with
the corrected `&` on the RHS (item 1).

## Second attempt — EXACT (2026-08-22, overnight run)

One second attempt using only the two documented levers:

1. Real 44-byte row aggregate (`Row { unsigned int link; unsigned char
   rest[40]; }`) declared on BOTH `D_8009D334` and `D_8009D310`, so cc1 kept
   the symbol in the store addressing and scaled `&D_8009D310[i+1]` by 44.
2. Address-only RHS `(unsigned int)&D_8009D310[i + 1]`.

Result on era `-O2 -G8` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`: size gate
passed at exactly `0x94`, fresh Docker build produced **EXACT SHA-1**
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `verify_us.sh` EXACT MATCH,
277 leaves. Candidate + registration left UNCOMMITTED for human review.
