# func_80042770 / func_80042964 — MATCHED (`LINK_EXACT`)

VRAM `0x80042770` / `0x80042964`, size `0x28` (10 words) each, file
`0x32F70` / `0x33164`. era flags `-O2 -G0` + **maspsx patch 5**
(`MASPSX_SYMBOL_AT_TEMP=1`, profile `era_o2_g0_symbol_at_temp`).

## Semantics

Two row readers over the stride-1048 (`0x418`) record table:

- `func_80042770(a0)` returns `D_800A0ED4[a0 * 1048] & 1`.
- `func_80042964(a0)` returns `D_800A0EDE[a0 * 1048]` (byte, zero-extended).

Stride `0x418` = 1048 bytes; the two globals are `0xA` bytes apart.

## Source

```c
extern unsigned char D_800A0ED4[];
int func_80042770(int a0) {
    return D_800A0ED4[a0 * 1048] & 1;
}
```

```c
extern unsigned char D_800A0EDE[];
int func_80042964(int a0) {
    return D_800A0EDE[a0 * 1048];
}
```

`src/func_80042770.c`, `src/func_80042964.c`.

## Lever — maspsx patch 5 (`symbol_at_temp`, the `$at` form with `%lo` kept)

Retail (10 words) uses the **`$at` temp with the `%lo` displacement left on
the load**:

```
lui   $at,%hi(D_800A0ED4)
addu  $at,$at,$v0
lbu   $v0,%lo(D_800A0ED4)($at)
```

The legacy maspsx expansion hammers the load to displacement `0(reg)`, and
patch 4 (`symbol_load_dest_temp`) uses the destination register instead of
`$at`; neither reproduces this shape. Patch 5 emits the `$at` temp while
preserving `%lo`, and is opt-in per leaf through
`era_o2_g0_symbol_at_temp`. Durable test
`tools/era/maspsx/tests/test_symbol_at_temp.py`, restored by
`scripts/setup_era.sh` `MASPSX_TRACKED` + `.gitignore` negation.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80042770.c 0x80042770 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad)
MISMATCHES=4  (HI16/LO16 placeholders)
MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py \
  src/func_80042770.c 0x80042770 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
# identical accounting for src/func_80042964.c 0x80042964 0x28
```

## Provenance

`configs/USA/disc1.yaml`:

```
- [0x32F70, c, func_80042770]
- [0x32F98, asm]
- [0x33164, c, func_80042964]
- [0x3318C, asm]
```

Both registered in `configs/USA/disc1_build_profiles.json` under
`era_o2_g0_symbol_at_temp`.
