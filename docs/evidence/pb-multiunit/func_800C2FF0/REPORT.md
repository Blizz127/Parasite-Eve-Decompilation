# func_800C2FF0 — landed (773 → 774)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x800C2FF0 · **file:** 0xB37F0 · **size:** 0xA8 (42 words)
**Unit:** B3390.s (mid-carve), prefix 0xB3390..0xB37F0, then func_800C3098
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_800C2FF0.c 0xB37F0 0xA8 --flags "-O2 -G0"` | `WORDS MATCH (+8 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (774 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`x = (arg0 & 0xFF) * 0x10`, `y = (arg1 & 0xFF) * 0x10`; then publish the
four rectangle corners into the `D_800F3310` halfword table:

```
3310: -x   3312: -y   3314: 0
3318:  x   331A: -y   331C: 0
3320: -x   3322:  y   3324: 0
3328:  x   332A:  y   332C: 0
```

`D_800F345C = arg0`, `D_800F345D = arg1`, then both are overwritten with
`arg - 1`. Returns `-y` in `$v0`.

## Levers

1. The scaled values and their negations are locals (`a0`, `a1`, `v1 = -a0`,
   `v0 = -a1`) reused across all twelve stores — retail keeps them in
   `$a0/$a1/$v1/$v0` and never recomputes.
2. `arg0 - 1` / `arg1 - 1` are computed at the top (into byte locals) and
   stored last; retail computes `addiu $a2,$a0,-1` / `addiu $a3,$a1,-1`
   before the first store and issues the two `sb` at the end.
3. Halfword stores of `int` locals reproduce retail's `sh` truncation.

## Divergences / negatives

* `-O2 -G8` = 23 diffs; `-O1 -G0` = 11.
* m2c draft `s16` temps for the negations also work, but the `int` locals
  give the cleanest match; the draft's trailing `return temp_v0` is
  equivalent to returning `v0`.
