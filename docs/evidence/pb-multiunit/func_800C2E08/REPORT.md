# func_800C2E08 — landed (774 → 775)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x800C2E08 · **file:** 0xB3608 · **size:** 0xA4 (41 words)
**Unit:** B3390.s (mid-carve), prefix 0xB3390..0xB3608, resume 0xB36AC..0xB37F0
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_800C2E08.c 0xB3608 0xA4 --flags "-O2 -G0"` | `WORDS MATCH (+12 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (775 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`do { ... } while (i < 0x40)` over the D_800F34F4 record table (stride 6).
When `*(signed char *)(D_800F34F4 + off + 1) != 0`: clear that byte,
`D_800E2248[6]--`, and
`found |= -((*(unsigned int *)(D_800E2248 + 4) & 0xFFFF0000) == 0x01000000)`.
Returns `found` (0 or -1), with `off` the strength-reduced `i * 6`.

## Levers (three independent, each worth 3-15 words)

1. **`D_800F34F4` as `unsigned int`, not a pointer.** `off + D_800F34F4`
   then materializes as `addu $v0,$a2,$a0` (retail); with a pointer type
   gcc normalizes to `addu $v0,$a0,$a2` (4 diffs).
2. **Stage the flag through a temp.** `int f = -(cond); found |= f;`
   compiles branchless (`and/xor/sltiu/negu/or` — retail). Writing
   `found |= -(cond)` directly makes cc1 if-convert to
   `bne $v0,$t0; nop; li $a3,-1` (15 diffs). `?:`, `0u - ...`,
   `if (...) found = -1` all if-convert too.
3. **Mask locals ordered between `i` and `off`.** Declaring
   `unsigned int mask = 0xFFFF0000; unsigned int want = 0x01000000;`
   after `i` and before `int off = 0;` makes the entry block emit
   `move $a3,0; move $a1,0; lui $t1,0xFFFF; lui $t0,0x100; move $a2,0`
   — retail's order. With the masks inline the `off = 0` init sinks
   before the two `lui` (3 diffs).

## Divergences / negatives

* `-O2 -G8` = 36 diffs; `-O1 -G0` = 17.
* `int off = i * 6;` and `int off; off = 0;` variants: 3 diffs.
