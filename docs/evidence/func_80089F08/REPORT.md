# func_80089F08 — LINK_EXACT

- Span: file `0x7A708` / VRAM `0x80089F08` / size `0x1C`.
- Split asm: `asm/disc1/7A510.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_80089F08.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80089F08.c 0x80089F08 0x1C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80089F08.c 0x80089F08 0x1C -O2 -G0
```

## Result

- `era_leaf_match.sh`: SIZE_MISMATCH C=0x20 ROM=0x1c; MISMATCHES=2 (both `%hi(D_8009B3FC)` immediates, resolved by the link).
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 28 bytes, target 0x1C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The 0x20-byte object carries a 4-byte `gas` alignment pad past the 0x1C body; the
linked `.text` is exactly 0x1C and byte-identical.

## Notes

Index base folded into `$a0` (`a0 = (a0 << 4) + (unsigned int)D_8009B3FC;`) keeps
the `0xC` displacement on the base register for the following `lhu`. Writing the
load as a single expression (`*(unsigned short *)((a0 << 4) + (unsigned int)
D_8009B3FC + 0xC)`) folds the addend into the address register instead.
`D_8009B3FC` is a **pointer global** (`extern unsigned short *`), not an array.
