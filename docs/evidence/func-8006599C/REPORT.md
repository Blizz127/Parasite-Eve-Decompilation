# func_8006599C — MATCHED (`LINK_EXACT`)

VRAM `0x8006599C`, size `0x2C` (11 words), file `0x5619C` in
`asm/disc1/55C00.s`. era flags `-O2 -G0` (YAML default).

## Semantics

Row halfword getter: returns the signed halfword at
`D_800B1624 + *(int *)(D_800B1624 + 0x10) + (a0 << 4) + 6`.

## Source

```c
extern unsigned char *volatile D_800B1624;
short func_8006599C(int a0) {
    return *(short *)(D_800B1624 + *(int *)(D_800B1624 + 0x10) + (a0 << 4) + 6);
}
```

`src/func_8006599C.c`.

## Lever — pointer-to-volatile base defeats the second-load CSE

Retail loads `D_800B1624` **twice** (two independent `lui`/`lw` pairs) before
using one as the row base and the other as the offset holder. A plain
`extern unsigned char *D_800B1624` lets cc1 CSE the second load into the
first register (`MISMATCHES=10`). Declaring the global
`unsigned char *volatile` keeps both loads, matching retail exactly
(`MISMATCHES` reduces to the four link-time HI16/LO16 placeholders).

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8006599C.c 0x8006599C 0x2C -O2 -G0
ROM  .text 44 bytes  C .text 48 bytes  target 44
SIZE_MISMATCH C=0x30 ROM=0x2C   (4 bytes trailing gas zero pad)
MISMATCHES=4  (two `lui %hi` + two `lw/addiu %lo` placeholders)
python3 tools/analysis/era_link_check.py src/func_8006599C.c 0x8006599C 0x2C -O2 -G0
linked .text 48 bytes, target 0x2C, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x5619C, c, func_8006599C]` (asm resumes at
`0x561C8`, immediately before the unregistered twin `func_800659C8` whose
store form is a different code shape).

An earlier, still-empty carve (`0x561C8` region) is untouched.
