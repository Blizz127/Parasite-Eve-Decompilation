# func_8001D268 — actor contact part scan (769 → 770)

- VRAM `0x8001D268`, file `0xDA68`, size `0xD8` (54 words), unit `ACAC`.
- Fresh-build evidence: `RESULT: EXACT MATCH`, candidate SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; `verify_us.sh`
  `VERIFY_US=PASS`, `matching-C count: 770 (from YAML)`.

## Offset math

The `[0xB118, asm]` run (resume after func_8001A890) was split at the
leaf:

```
[0xB118, asm]              prefix, 0xB118..0xDA68
[0xDA68, c, func_8001D268] leaf,   0xDA68..0xDB40 (0xD8)
[0xDB40, asm]              resume, 0xDB40..0x116FC
```

## Compiler profile

Default `era_o2_g0` (`-O2 -G0`, maspsx 2.21 `--dont-expand-li`). The
function only touches memory through its arguments, so no symbol/gp
addressing is involved and no environment gate is needed.

## What diverged and how it was fixed

The m2c draft was structurally right; three codegen choices had to be
forced by source shape:

1. `((record[0] >> 21) & 7) < 3` produced `sltiu` because the shifted
   value is unsigned. Retail uses signed `slti` (`28420003`), so the
   comparison is written `(int)((record[0] >> 21) & 7) >= 3`.
2. Retail zeroes the loop counter `$a1` before sign-extending `part`
   (`addu $a1,$0,$0` then `sra $a3,$v0,16`); writing `i = 0;` before
   `part = (short)part;` reproduces that order.
3. The match branch must compare with `part` as the first operand
   (`bne $a3,$v0`); `if (part == value)` gives that operand order.

The `j` at +0x44 targets the shared epilogue label and is resolved by the
linker, so the raw-object word differs from retail and matches after the
build.

## Authoritative checks

```
$ bash scripts/split_us.sh
disc1 plan: 1115 spans (770 c, 343 asm, 2 rodata), geometry=0x1EE000
$ bash scripts/build_us.sh
RESULT: EXACT MATCH
Matching claim: YES (770 registered C leaves)
$ bash scripts/verify_us.sh
VERIFY_US=PASS
```
