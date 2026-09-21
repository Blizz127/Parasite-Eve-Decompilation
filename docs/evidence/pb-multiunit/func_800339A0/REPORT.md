# func_800339A0 — landed (768 → 769)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x800339A0 · **file:** 0x241A0 · **size:** 0x80 (32 words)
**Unit:** 20EE0.s (tail), immediately before already-matched func_80033A20 (0x24220)
**Profile:** `era_o2_g8` (era gcc 2.7.2, `-O2 -G8`)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_800339A0.c 0x241A0 0x80 --flags "-O2 -G8"` | `WORDS MATCH` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (769 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

Copies the 16 bytes at D_80010E38 (4 halfword pairs: ddd0?/… `dd00 b100
0f00 b100 0f00 0f00 dd00 0f00`) onto the local stack frame, indexes the copy
at `(style & 0xFF) * 4`, and publishes the selected pair through the
gp-relative battle state:

* `row[0]` → `D_8009CE84` (gp+0x114)
* `row[1]` → `D_8009CE86` (gp+0x116)
* `style`  → `D_8009CE80` (gp+0x110, byte)

The `row[1]` value is left in `$v0` at return (void function; the callers
ignore it). gp base 0x8009CD70 (see `pc_port/tools/pe_btl6_339a0_oracle.py`).

## Levers

1. **`-O2 -G8`, not the `-G0` default.** D_8009CE80/84/86 sit in the
   small-data window (gp+0x110/0x114/0x116). `-G0` emits `lui $at` +
   `%lo($at)`; `-G8` emits retail's direct `sb/sh %lo($gp)` (R_MIPS_GPREL16).
2. **`__builtin_memcpy(buf, D_80010E38, 16)` on a `char *` destination**
   reproduces the 16-byte unaligned block exactly: `lwl/lwr` ×4 + `swl/swr`
   ×4, including the scheduling nop before the fourth `swl`.
3. **Local `unsigned char *row` cursor, not `unsigned short *`.** The
   `unsigned short *` row form allocates the pointer to `$v0` and loads into
   `$v1`; the byte cursor form keeps the pointer in `$v1` and the loaded
   values in `$v0`, matching retail.
4. **Store order `row[0]; row[1]; style;`** (not `row[0]; style; row[1]`).
   The `style` byte store must sink past both halfword loads — the natural
   source order (matching the m2c draft) hoists `sb` one slot too early.

## Divergences / negatives

* m2c draft (`build/auto_leaves/func_800339A0.c`) emits `M2C_ERROR` for the
  `lwr` pairs and `saved_reg_gp->unk` for the gp stores: unusable as-is.
* auto_leaf swept 12 profiles, none matched (the two structural levers above
  are source-shape, not flag, choices).
* `unsigned short *row` / `short *row` / `struct {u16 a,b;}*` all produce the
  wrong v0/v1 split (6 diffs); `int i` index form produces 9.
