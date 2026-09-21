# func_800CCA40 — LINK_EXACT

- Span: file `0xBD240` / VRAM `0x800CCA40` / size `0x38` (14 words).
- Split asm: `asm/disc1/BC7C4.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_800CCA40.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_800CCA40.c 0x800CCA40 0x38 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800CCA40.c 0x800CCA40 0x38 -O2 -G0
```

## Result

- `era_leaf_match.sh`: `SIZE_MISMATCH C=0x40 ROM=0x38` with
  `BYTE_EXACT (ignoring gas align pad)` — every ROM word matches; the object carries
  one 4-byte `gas` alignment pad.
- Link level:

```
linked .text 56 bytes, target 0x38, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes

Byte-counter variant: the stepped field is the **byte** at `a2+3` (`lbu`), stepped
down by 2, and re-read as `signed char` (`lb`) for the `slti 0x1E` test. The `+4`
halfword takes the `+=` step instead. On the arm `a1[1] = 2` (the `+3` byte is *not*
zeroed here).

STEP for this leaf: **`2`** (paired with `func_800CCA40`/`func_800CCA78`).

Contiguous carve: prefix `0xBD240`, `0x38` + `0x38`, resume at `0xBD2B0`. The
`0x38`-byte sibling `func_800CC92C` (a three-field stepping variant) was attempted
and PARKED — no C spelling reproduced retail's field-issue order.
