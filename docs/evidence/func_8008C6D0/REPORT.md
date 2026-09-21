# func_8008C6D0 — LINK_EXACT

- Span: file `0x7CED0` / VRAM `0x8008C6D0` / size `0x3C`.
- Split asm: `asm/disc1/7CA90.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_8008C6D0.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8008C6D0.c 0x8008C6D0 0x3C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8008C6D0.c 0x8008C6D0 0x3C -O2 -G0
```

## Result

- `era_leaf_match.sh`: SIZE_MISMATCH C=0x40 ROM=0x3c; MISMATCHES=4 (`D_800B8BB4`/`D_8009D2B8` `%hi`
immediates, resolved by the link).
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 60 bytes, target 0x3C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The 4-byte object pad is `gas` alignment; the linked `.text` is exactly 0x3C.

## Notes

`D_8009D2B8` is seeded from `a0+4`, then the first word of each of 0x18 records
strided `0x11C` in `D_800B8BB4` gets `|= 3`. The counter must be **`unsigned int`**
in a `do/while` (a signed `int` counter emits `slt`, retail has `sltiu 0x18`).
