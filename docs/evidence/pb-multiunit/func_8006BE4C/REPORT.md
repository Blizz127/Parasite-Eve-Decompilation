# func_8006BE4C — landed (769 → 770)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x8006BE4C · **file:** 0x5C64C · **size:** 0x80 (32 words)
**Unit:** 5B1E4.s (mid-carve), prefix 0x5B1E4..0x5C64C, resume 0x5C6CC..0x5E348
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_8006BE4C.c 0x5C64C 0x80 --flags "-O2 -G0"` | `WORDS MATCH` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (770 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US_PASS` |

## Semantics

If the live mount state byte `D_800B0CE2` is in 0xA..0xE (unsigned
`D_800B0CE2 - 0xA < 5`), then:

* when it differs from the previous state byte `D_800B0CE3`, set bit
  0x200000 in the flag word `D_800B0CD8`;
* independently, when `(state - 0xA) >> 1` differs from the signed
  `(prev - 0xA) / 2`, set bit 4 in the byte flag `D_800B0CE6`.

Returns 0.

## Levers

1. **`unsigned int *p = &D_800B0CD8;` then `*p |= 0x200000;`** — retail
   materializes the base once (`lui $a2,%hi; addiu $a2,%lo`) and uses
   `lw/sw 0($a2)`. A plain global read-modify-write makes cc1 emit
   `lui $v0,%hi; lw %lo($v0)` and `lui $at,%hi; sw %lo($at)` — two extra
   words that shift both branch targets (19 diffs). This is the same
   "shared symbolic base" lever used by func_8006DB48 in the same unit.
2. `(int)(D_800B0CE3 - 0xA) / 2` must stay *signed*: retail's `srl
   $a0,$v1,31; addu; sra` is the /2 sign-fixup sequence.

## Divergences / negatives

* m2c draft already had the right control flow; only the addressing mode
  and the local pointer were missing.
* `-O2 -G8`, `-O1 -G0`, and `MASPSX_THREE_WORD_SYMBOL_STORE=1` all fail
  (the symbols are outside the gp window: 0x800B0CE2 > gp base + 0x7FF0).
