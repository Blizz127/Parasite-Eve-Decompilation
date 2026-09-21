# func_8005DB8C — MATCHED (`LINK_EXACT`)

VRAM `0x8005DB8C`, size `0x20` (8 words), file `0x4E38C` in
`asm/disc1/4E2FC.s`. era flags `-O2 -G0` (YAML default). On-path fan-in 9.

## Semantics

Returns `D_800A8038 + (a0 << 9) + (&D_800A8038 - 0x10)` — the record base for
slot `a0` biased by `-0x10`, added to the global head word.

## Source

```c
extern int D_800A8038;
int func_8005DB8C(unsigned int a0) {
    unsigned char *base = (unsigned char *)&D_800A8038;
    unsigned int off = a0 << 9;
    unsigned char *q = base - 0x10;
    unsigned char *p = off + (unsigned int)q;
    return *(int *)base + (int)p;
}
```

`src/func_8005DB8C.c`.

## Lever — data symbol plus two separate address locals

Retail materialises the address as an `addiu` (`lui $v0,%hi` /
`addiu $v0,$v0,%lo`), not a `lui`+`ori` **constant** — so `D_800A8038` must be
referenced as a *data symbol* (`&D_800A8038`), the same lever proven for the
boot-spine/display cluster. Two address locals are load-bearing: `q = base -
0x10` gives retail's `addiu v1,v0,-16`, and declaring `off` *before* `q`
supplies the `$a0` shift first so `$v0` stays free for the `lw`
(`MISMATCHES=2`, both `%hi`/`%lo` placeholders). Folding `-0x10` into the
later `addu` and reusing one pointer gives `MISMATCHES=4` and a different
`$v1` reload.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_8005DB8C.c 0x8005DB8C 0x20 -O2 -G0
ROM  .text 32 bytes  C .text 32 bytes  target 32
MISMATCHES=2 first_off=0  (lui %hi / addiu %lo placeholders)
python3 tools/analysis/era_link_check.py src/func_8005DB8C.c 0x8005DB8C 0x20 -O2 -G0
linked .text 32 bytes, target 0x20, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x4E38C, c, func_8005DB8C]` (asm resumes at
`0x4E3AC`, immediately before the previously-carved `func_8005DBF8`).
