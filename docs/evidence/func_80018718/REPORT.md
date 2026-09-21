# func_80018718 — LINK_EXACT

- Span: file `0x8F18` / VRAM `0x80018718` / size `0x3C`.
- Split asm: `asm/disc1/8D98.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_80018718.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80018718.c 0x80018718 0x3C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80018718.c 0x80018718 0x3C -O2 -G0
```

## Result

- `era_leaf_match.sh`: SIZE_MISMATCH C=0x40 ROM=0x3c; MISMATCHES=3 (the `lui`/`lw` pair for `D_800A76C4`
and one branch immediate, resolved by the link).
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 60 bytes, target 0x3C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The 4-byte object pad is `gas` alignment; the linked `.text` is exactly 0x3C.

## Notes

Same gate shape as `func_80016FE0` with bit 2 of `D_800A76C4` and the 1/0
outputs swapped: the `if (...) *out = 1; else *out = 0;` spelling is what matches
(an early-return arm perturbs the branch/layout).
