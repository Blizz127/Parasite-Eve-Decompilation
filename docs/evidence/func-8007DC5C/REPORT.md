# func_8007DC5C / func_8007DC84 — MATCHED (`LINK_EXACT`)

VRAM `0x8007DC5C` / `0x8007DC84`, size `0x28` (10 words) each, file
`0x6E45C` / `0x6E484`. era flags `-O2 -G0` (YAML default).

## Semantics

Field-insert setters on the color word behind the `D_8009B410` pointer
global: clear bits 24-27 and OR in a constant.

- `func_8007DC5C` ORs `0x20000000` (sets bit 29).
- `func_8007DC84` ORs `0x22000000` (sets bits 29 and 25).

## Source

```c
extern unsigned int *D_8009B410;

void func_8007DC5C(void) {
    *D_8009B410 = (*D_8009B410 & 0xF0FFFFFF) | 0x20000000;
}
```

```c
extern unsigned int *D_8009B410;

void func_8007DC84(void) {
    *D_8009B410 = (*D_8009B410 & 0xF0FFFFFF) | 0x22000000;
}
```

`src/func_8007DC5C.c`, `src/func_8007DC84.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8007DC5C.c 0x8007DC5C 0x28 -O2 -G0
ROM  .text 40 bytes  C .text 48 bytes  target 40
SIZE_MISMATCH C=0x30 ROM=0x28   (8 bytes trailing gas zero pad)
MISMATCHES=2  (the `lui %hi` / `lw %lo` placeholders on D_8009B410)
python3 tools/analysis/era_link_check.py src/func_8007DC5C.c 0x8007DC5C 0x28 -O2 -G0
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
# identical accounting for src/func_8007DC84.c 0x8007DC84 0x28
```

## Provenance

`configs/USA/disc1.yaml`:

```
- [0x6E45C, c, func_8007DC5C]
- [0x6E484, c, func_8007DC84]
```

Both YAML-default profile.
