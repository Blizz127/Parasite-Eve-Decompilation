# func_800CD5B0 — LINK_EXACT

- Span: file `0xBDDB0` / VRAM `0x800CD5B0` / size `0x3C` (15 words).
- Split asm: `asm/disc1/BDDB0.s`.
- Profile: `era_o2_g0` (`-O2 -G0`).
- Source: `src/func_800CD5B0.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_800CD5B0.c 0x800CD5B0 0x3C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_800CD5B0.c 0x800CD5B0 0x3C -O2 -G0
```

## Result

- `era_leaf_match.sh`: `SIZE_MISMATCH C=0x40 ROM=0x3C` with
  `BYTE_EXACT (ignoring gas align pad)` — every ROM word matches; the object carries
  one 4-byte `gas` alignment pad.
- Link level:

```
linked .text 60 bytes, target 0x3C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes

Third member of the countdown-timer family (see `docs/evidence/func_800C8C80`), but
with the **byte** counter field: `a2+3` steps down by 8 and is re-read as
`signed char` for the `slti 0x14` test; `the `+6` halfword` takes the `+= 0x3C` step. On the
arm the `+3` byte is zeroed and `a1[1]` is set to 2.

Standard halfword family, but the **down-step is `-0xA`** (`*(unsigned short *)
(a2+4) -= 0xA`), not `-8`. Carve: prefix `0xBDDB0`, `0x3C`, resume at `0xBDDEC`.
