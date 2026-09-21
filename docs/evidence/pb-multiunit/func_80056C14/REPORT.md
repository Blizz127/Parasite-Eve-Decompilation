# func_80056C14 — landed (778 → 779)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80056C14 · **file:** 0x47414 · **size:** 0x2C (11 words)
**Unit:** 4680C.s (mid-carve), prefix 0x4680C..0x47414, resume 0x47440..0x48370
**Profile:** `era_o2_g0_passthrough_load` (`-O2 -G0` + `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1`)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80056C14.c 0x47414 0x2C --flags "-O2 -G0" --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1` | `WORDS MATCH (+4 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (779 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`return arg0 < 3 ? D_800A1E6E[arg0 << 4] : 0;` — halfword read at
D_800A1E6E + (arg0 << 5).

## Levers

1. **`MASPSX_PASSTHROUGH_SYMBOL_LOAD=1`** (profile
   `era_o2_g0_passthrough_load`). Without it cc1 emits
   `lui $at,%hi; addiu $at,$at,%lo; addu $at,$at,$v0; lhu $v0,0($at)`
   (4-word address materialization, 4-6 diffs). With it the low half
   folds into the indexed load:
   `lui $at,%hi; addu $at,$at,$v0; lhu $v0,%lo($at)` — retail.
2. Index scaling `D_800A1E6E[arg0 << 4]` (halfword elements) gives
   retail's `sll $v0,$a0,5`.
3. Ternary form; the block form and `if (arg0 >= 3) return 0;` are
   equivalent once the passthrough knob is on.

## Divergences / negatives

* Without the passthrough env: 4 diffs (item 1).
* The +4 pad bytes are a trailing alignment nop, trimmed by the build.
