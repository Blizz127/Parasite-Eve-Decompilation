# func_80066BD8 — landed (775 → 776)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80066BD8 · **file:** 0x573D8 · **size:** 0xA4 (41 words)
**Unit:** 56438.s (mid-carve), prefix 0x56438..0x573D8, resume 0x5747C..0x5AA5C
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80066BD8.c 0x573D8 0xA4 --flags "-O2 -G0"` | `WORDS MATCH (+12 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (776 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

Snapshot `D_800BCFE8/EA/EC`; store the new window `arg1/arg2/arg3`;
`D_800BCFEE = 2`; `D_800BCFF6 = arg0`; `D_800BCFF8 = 0`; copy the old
words into `D_800BCFF0/2/4`. When `(arg4 & 0xFFFF) < 4` and `>= 0` set
`D_800BCFEF = arg4`, else `1`. Returns 0.

## Levers

1. **`unsigned short arg4` + `int v = arg4 & 0xFFFF`.** Retail loads the
   fifth (stack) argument with `lhu $t2,0x10($sp)` and compares the masked
   value with signed `slti` plus a `bltz`. A `short`/`int` arg4 changes the
   load (14 diffs); an `unsigned short v` drops the `slti`/`bltz` shape
   (10 diffs).
2. **Nested `if` with an early `return 0` on the success path.** Retail's
   `bltz $v1` targets the same store-1 block as the `v < 4` failure. The
   flat `if (v < 4 && v >= 0) ... else ...` drops the `bltz` entirely
   (10 diffs); a nested form with a duplicated else makes `bltz` jump to
   the epilogue instead (1 diff: branch offset `04600005` vs `04600008`).
3. **`D_800BCFE8`-style absolute globals** — all are above the gp window,
   so `-G0` emits retail's `lui $at,%hi; sh %lo($at)` sequence.

## Divergences / negatives

* `-O2 -G8` = 23 diffs; `-O1 -G0` = 15.
* `if (!(v < 4 && v >= 0)) { 1 } else { arg4 }`: 10 diffs.
