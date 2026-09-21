# func_800C3098 — landed (772 → 773)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x800C3098 · **file:** 0xB3898 · **size:** 0x9C (39 words)
**Unit:** B3390.s (mid-carve), prefix 0xB3390..0xB3898, resume 0xB3934..0xB6D3C
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_800C3098.c 0xB3898 0x9C --flags "-O2 -G0"` | `WORDS MATCH (+4 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (773 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`switch ((short)arg0)`: case 0x10 → `D_800F33AC = 0`; case 0x100 →
`D_800F33AC = 1`; default → `func_80071A74(&D_800C2110)`. Then
`D_800E27AC = func_80077A64(D_800F33AC, D_800E224C, D_800F3424, D_800F3426)`.

## Levers

1. **Void return.** Retail is
   `jal func_80077A64; nop; lui $at,%hi(D_800E27AC); sh $v0,%lo($at)` —
   the callee's `$v0` is stored and left in place. Returning
   `unsigned short` adds `andi $v0,$v0,0xFFFF` (10 diffs); returning
   `int` with a `short` temp adds `move/sll/sra` (11 diffs).
2. `(short)arg0` gives retail's `sll $a0,$a0,16; sra $a0,$a0,16` prologue.
3. The `sw $ra,0x10($sp)` sits in the first `beq` delay slot (matched by
   the natural switch codegen).

## Divergences / negatives

* `-O2 -G8` = 16 diffs; `-O1 -G0` = 10.
* `D_800C2110` only appears as the `lui/addiu` argument address here.
