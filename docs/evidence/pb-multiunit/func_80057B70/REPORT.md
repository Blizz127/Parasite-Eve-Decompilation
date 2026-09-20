# func_80057B70 — landed (777 → 778)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80057B70 · **file:** 0x48370 · **size:** 0xA0 (40 words)
**Unit:** 4680C.s (mid-carve), prefix 0x4680C..0x48370, then func_80057C10
**Profile:** `era_o2_g8` (added to the assignment list)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80057B70.c 0x48370 0xA0 --flags "-O2 -G8"` | `WORDS MATCH` (no pad) |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (778 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`v = func_8005DB44(arg0 + 0xEB)`. If the gp flag `D_8009D028`
(gp+0x2B8) is nonzero: `s = func_800579D4(arg0, func_800515F8(0))`;
`p = func_80051098()`; `p[0]=1; p[1]=arg0; p[2]=s`; return
`func_800512AC(1, &arg0)` (address of a stack copy of arg0). Else if
`v[0xE] == 1` return `func_80051770(arg0)`, else return 1.

## Levers

1. **`-O2 -G8`.** `D_8009D028` is at gp+0x2B8, so retail uses
   `lw $v1,0x2B8($gp)`. `-G0` emits an absolute `lui/lw` pair and also
   mis-schedules the frame (25 diffs).
2. The stack copy `sp10 = arg0; return func_800512AC(1, &sp10);`
   reproduces retail's `sw $s1,0x10($sp)` in the call's delay slot.
3. `if (v[0xE] == 1) return func_80051770(arg0); return 1;` gives
   retail's `li $v0,1` before the `bne`.

## Divergences / negatives

* `-O2 -G0` = 25 diffs; `-O1 -G8` = 31.
* m2c draft's `saved_reg_gp->unk2B8` was the only missing piece — the
  draft's control flow was already exact.
