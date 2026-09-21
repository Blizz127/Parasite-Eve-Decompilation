# func_800751E4 — matching C leaf

VRAM `0x800751E4`, size `0xC8` (50 words), era `-O2 -G0` plus maspsx patch 3
(`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`). Source: `src/func_800751E4.c`.
YAML carve `0x659E4` in `configs/USA/disc1.yaml`.

## Semantics

`D_8009574E`/`D_80095748` gate + log wrapper around a **next-pointer link
walk**, then a display-window seed:

- if `D_8009574E >= 2`, log `(&D_800118F8, a0, a1)` through `D_80095748`;
- walk `n = a1 - 1` nodes: clear `p[3]`, then splice the low 24 bits of
  `(p + 4)` (the next-node pointer) into the word at `p` while keeping
  retail's high byte (`p` is a 0xA4-byte record header):
  `*(int*)p = (*(int*)p & 0xFF000000) | ((p+4) & 0xFFFFFF)`;
- after the walk, seed the `D_8009580C` / `D_800957F8` window pair:
  `*(int*)&D_8009580C = ((unsigned)&D_800957F8 & 0xFFFFFF) | 0x4000000;`
  `*(int*)p = (unsigned)&D_8009580C & 0xFFFFFF;` and return `p`.

`retail 0x0C` is `lbu`-first with `s0`=a0 written before `s1`=a1; the pinned
locals `register unsigned char *s0 asm("$16")` / `register int s1
asm("$17")` reproduce that save order plus the `bnez` condition
(`sltiu v0,v0,2` then `bnez`, i.e. the source is `if (D_8009574E >= 2) …`).

The mask merge needs `lo`/`hi` named locals *inside* the loop; writing the
`&`/`|` operands the other way (`next & lo` first) moved the `ori` into
`$a0`/`$v0` and perturbed the loop register set.

## Commands

```
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 tools/analysis/era_leaf_match.sh \
    src/func_800751E4.c 0x800751E4 0xC8 -O2 -G0
MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3 tools/analysis/era_link_check.py \
    src/func_800751E4.c 0x800751E4 0xC8 -O2 -G0
```

## Object accounting

`SIZE_MISMATCH C=0xd0 ROM=0xc8`, `MISMATCHES=10`, every mismatch is a
HI16/LO16 relocation field whose literal is zeroed in the relocatable object
and resolved by the linker:

```
0x800751e4  ROM 3c028009  C 3c020000   lui  v0,%hi(D_8009574E)
0x800751e8  ROM 9042574e  C 90420000   lbu  v0,%lo(D_8009574E)(v0)
0x8007520c  ROM 3c028009  C 3c020000   lui  v0,%hi(D_80095748)
0x80075210  ROM 8c425748  C 8c420000   lw   v0,%lo(D_80095748)(v0)
0x80075214  ROM 3c048001  C 3c040000   lui  a0,%hi(D_800118F8)
0x80075218  ROM 248418f8  C 24840000   addiu a0,%lo(D_800118F8)
0x80075270  ROM 3c058009  C 3c050000   lui  a1,%hi(D_8009580C)
0x80075274  ROM 24a5580c  C 24a50000   addiu a1,%lo(D_8009580C)
0x80075278  ROM 3c038009  C 3c030000   lui  v1,%hi(D_800957F8)
0x8007527c  ROM 246357f8  C 24630000   addiu v1,%lo(D_800957F8)
```

The `0x40` trailing pad is gas alignment and is zero.

## Link-level

`linked .text 208 bytes, target 0xc8, word mismatches=0, nonzero_pad=0` →
`LINK_EXACT`. The three data symbols resolve to the addresses retail
touches (`D_8009574E` `0x8009574E`, `D_80095748` `0x80095748`,
`D_800118F8` `0x800118F8`, `D_8009580C` `0x8009580C`, `D_800957F8`
`0x800957F8`).
