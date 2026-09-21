# func_80016FE0 — LINK_EXACT

- Span: file `0x77E0` / VRAM `0x80016FE0` / size `0x38`.
- Split asm: `asm/disc1/7640.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_80016FE0.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80016FE0.c 0x80016FE0 0x38 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80016FE0.c 0x80016FE0 0x38 -O2 -G0
```

## Result

- `era_leaf_match.sh`: SIZE_MISMATCH C=0x40 ROM=0x38; MISMATCHES=3 (the `lui`/`lw` pair for `D_8009D2E8`
and one branch immediate, all resolved by the link).
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 56 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The 8-byte object pad is `gas` alignment; the linked `.text` is exactly 0x38.

## Notes

`if (D_8009D2E8 & 1) *out = 0; else *out = 1;` reproduces retail exactly. The
early-return spelling (`if (...) {{ ...; return 1; }}`) reverses the branch and
duplicates the `li v0,1`, changing the layout.
