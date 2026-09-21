# func_80076B98 — MATCHED (`LINK_EXACT`)

VRAM `0x80076B98`, size `0x48` (18 words), file `0x67398` in
`asm/disc1/66B54.s`. era flags `-O2 -G0` (YAML default).

## Context — carved out of the oversized `func_80076B58` span

This function was previously **swallowed** by the `func_80076B58` `c` span,
which was declared `0x88` while its C compiles to `0x40`. The deep preflight
(`disc1_preflight.py --deep`) reported:

```
FAIL [deep-size] func_80076B58: declared span 0x88 (0x67358->0x673E0)
exceeds compiled .text 0x40 by 0x48
```

Retail confirms the real boundary: `func_80076B58` ends with
`jr $ra / addu $v0,$zero,$zero` at `0x80076B94`, and `func_80076B98` begins at
`0x80076B98` and ends with `jr $ra` at `0x80076BD8` (`sw` in the delay slot).
So the true sizes are `func_80076B58` = `0x40` and `func_80076B98` = `0x48`.

## Semantics

Four-word control store: `*D_80095854 = 0x4000002`, `*D_80095858 = a0`,
`*D_8009585C = 0`, `*D_80095860 = 0x1000401`.

## Source

```c
extern unsigned int *D_80095854;
extern unsigned int *D_80095858;
extern unsigned int *D_8009585C;
extern unsigned int *D_80095860;
void func_80076B98(unsigned int a0) {
    *D_80095854 = 0x4000002;
    *D_80095858 = a0;
    *D_8009585C = 0;
    *D_80095860 = 0x1000401;
}
```

`src/func_80076B98.c`.

## Lever — the fourth constant needs its own local

Retail materialises `0x1000401` as a **separate** `lui $v1,0x100` before the
final `ori` (in the `jr` delay slot), rather than hoisting it. Giving the
fourth assignment its own named `v`/`*D_80095860 = <local>` is what keeps the
`lui` in place; using a hoisted named local for `a0` instead perturbs the
register order (`MISMATCHES=8`).

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80076B98.c 0x80076B98 0x48 -O2 -G0
ROM  .text 72 bytes  C .text 80 bytes  target 72
SIZE_MISMATCH C=0x50 ROM=0x48   (8 bytes trailing gas zero pad)
MISMATCHES=8  (lui/addiu/ori %hi/%lo placeholders)
python3 tools/analysis/era_link_check.py src/func_80076B98.c 0x80076B98 0x48 -O2 -G0
linked .text 80 bytes, target 0x48, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x67398, c, func_80076B98]` sits immediately
after the corrected `- [0x67358, c, func_80076B58]`.
