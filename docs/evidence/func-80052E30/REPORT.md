# func_80052E30 — MATCHED (`LINK_EXACT`)

VRAM `0x80052E30`, size `0x80` (32 words), file `0x43630` in
`asm/disc1/43408.s`. era flags `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS`
(`D_800A1F84,D_800C0E48,D_8009D05C`), profile
`era_o2_g8_force_descriptor_absolute`.

## Semantics

Dispatch-config selector, on-path fan-in 7. If `a0 != 0` **and** the live
DMA/stream pointer `D_8009D04C` is set, it installs the `D_800A1F84`
descriptor and caps the config block at 4; otherwise it installs
`D_800C0E48`, calls `func_80052F70` for the new length, installs
`D_8009D05C` and caps at 2. The config block is the gp-relative
`D_8009D048..D_8009D064`.

## Source

```c
extern int D_8009D048;
extern int D_8009D04C;
extern int D_8009D050;
extern int D_8009D054;
extern int D_8009D058;
extern int D_8009D064;
extern int D_800A1F84;
extern int D_800C0E48;
extern int D_8009D05C;
extern int func_80052F70();

void func_80052E30(int a0) {
    if (a0 != 0 && D_8009D04C != 0) {
        int v = D_8009D054;
        D_8009D048 = D_8009D04C;
        D_8009D058 = (int)&D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = v;
    } else {
        D_8009D048 = (int)&D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (int)&D_8009D05C;
        D_8009D064 = 2;
    }
}
```

`src/func_80052E30.c`.

Retail keeps the raise/lower order: `D_8009D054` is loaded *first*, then
`D_8009D048`, then the two pointers, then `D_8009D064 = 4`, then
`D_8009D050 = v` last. Reordering those two stores changes the schedule.

## Load-bearing profile — absolute descriptor pointers with gp-relative block

The six config words are gp-relative (`sw/lw 0x2D8..0x2F4($gp)`), so the leaf
needs `-G8`. But the three descriptor *addresses* are outside the small-data
window: with `-G8` alone the link aborts with

```
small-data section too large; lower small-data size limit (see option -G)
relocation truncated to fit: R_MIPS_GPREL16 against `D_800C0E48'
```

`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800A1F84,D_800C0E48,D_8009D05C` strips their
`.extern` so maspsx/as materialize them with `lui`/`addiu` absolute relocs
while the six config words stay gp-relative — exactly retail's split.

```
-O2 -G8                                   object MISMATCHES=27
-O2 -G8 + force-absolute symbols           link-level mismatches=0
-O2 -G0                                   MISMATCHES=30
```

The object-level `MISMATCHES=27` are relocation slots (the object carries
`%hi/%lo` against forced-absolute symbols where the split asm text has already
resolved ones); the link-level check at the retail VMA is authoritative.

## Commands

```
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800A1F84,D_800C0E48,D_8009D05C \
  tools/analysis/era_leaf_match.sh src/func_80052E30.c 0x80052E30 0x80 -O2 -G8
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800A1F84,D_800C0E48,D_8009D05C \
  python3 tools/analysis/era_link_check.py src/func_80052E30.c 0x80052E30 0x80 -O2 -G8
linked .text 128 bytes, target 0x80, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x43630, c, func_80052E30]`. Profile
`era_o2_g8_force_descriptor_absolute`; `profile_necessity.py --only
func_80052E30` reports OK.
