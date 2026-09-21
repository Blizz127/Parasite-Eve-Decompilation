# func_800C8CBC — LINK_EXACT

- Span: file `0xB94BC` / VRAM `0x800C8CBC` / size `0x3C` (`15` words).
- Split asm: `asm/disc1/B9480.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_800C8CBC.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_800C8CBC.c 0x800C8CBC 0x3C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800C8CBC.c 0x800C8CBC 0x3C -O2 -G0
```

## Result

- `era_leaf_match.sh`: `SIZE_MISMATCH C=0x40 ROM=0x3C` with
  `BYTE_EXACT (ignoring gas align pad)` — every ROM word matches; the object carries
  one 4-byte `gas` alignment pad.
- Link level (retail VMA, all undefined symbols resolved):

```
linked .text 60 bytes, target 0x3C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes

Countdown-timer step. `*(unsigned short *)(a2+4) -= 8;` (retail `lhu`+`addiu`+`sh`),
then `*(unsigned short *)(a2+6) += STEP;`, then **a second, independent** `short`
read of `a2+4` for the `slti 0x14` test (`if (*(short *)(a2 + 4) < 0x14) {{ ... }}`).
Folding the compare into the value already in hand changes the register home; the
reload is load-bearing. On the arm the `+4` field is zeroed and the state byte
`a1[1]` is set to 2.

STEP for this leaf: **`0x78`**.

Triplet with `func_800C8C80`/`func_800C8CBC`/`func_800C8CF8` — identical body,
differing only in the `+6` step constant. Contiguous carve: prefix `0xB9480`,
`0x3C` + `0x3C` + `0x3C`, resume at `0xB9534`.
