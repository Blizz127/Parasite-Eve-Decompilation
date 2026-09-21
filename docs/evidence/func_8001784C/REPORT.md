# func_8001784C — LINK_EXACT

- Span: file `0x804C` / VRAM `0x8001784C` / size `0x30`.
- Split asm: `asm/disc1/7640.s`.
- Profile: `era_o2_g8` (`-O2 -G8`).
- Source: `src/func_8001784C.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8001784C.c 0x8001784C 0x30 -O2 -G8
python3 tools/analysis/era_link_check.py src/func_8001784C.c 0x8001784C 0x30 -O2 -G8
```

## Result

- `era_leaf_match.sh`: MISMATCHES=2 (only the `%hi(D_8009D300)` immediates; the linked image is exact).
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

Object and ROM are both 0x30 bytes (no alignment pad).

## Notes

`D_8009D300` is a **pointer global** (`extern unsigned int *`) reached gp-relative
at `0x590($gp)`; only the `-G8` rung keeps it in the small-data section. Under
`-G0` the loads become absolute `lui %hi`/`lw %lo` sequences and the object grows
to 0x40 bytes.
