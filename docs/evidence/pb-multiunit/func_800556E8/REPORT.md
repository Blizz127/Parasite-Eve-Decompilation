# func_800556E8 — landed (780 → 781)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x800556E8 · **file:** 0x45EE8 · **size:** 0x3C (15 words)
**Unit:** 44AA0.s (mid-carve), prefix 0x44AA0..0x45EE8, resume 0x45F24..0x467B4
**Profile:** NEW `era_o2_g8_passthrough_load` (`-O2 -G8` + `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1`)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_800556E8.c 0x45EE8 0x3C --flags "-O2 -G8" --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1` | `WORDS MATCH (+4 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (781 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`if (a0 < 0) return 0; if (a0 >= D_8009D040) return 0; return D_800A1D9C[a0];`
— signed halfword table read with a gp-relative count bound.

## Levers

1. **New profile `era_o2_g8_passthrough_load`.** This leaf needs BOTH
   knobs: `-G8` for the gp count load (`lw $v0,0x2D0($gp)`) and
   `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1` to fold the table's `%lo` into the
   indexed `lh` (`lui $at,%hi; addu $at,$at,$v0; lh $v0,%lo($at)`).
   `-G8` alone = 7 diffs; `-G0` + passthrough = 9; `-G0` alone = 10.
2. `short D_800A1D9C[]` (not `unsigned short`) gives retail's `lh`.
3. `D_8009D040` (gp+0x2D0, `int`) is the bounds count.

## Divergences / negatives

* The +4 pad bytes are a trailing alignment nop, trimmed by the build.
