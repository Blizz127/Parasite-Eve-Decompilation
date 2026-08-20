# func_80030534 — 2D distance helper matching C (20 words)

```text
MATCHING_C — func_80030534 era -O2 -G0 + aspsx 2.30, 20/20 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80030534..0x80030584 exclusive
file        0x20D34 size 0x50
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 231
func_80030534.c.o .text: 0x50→0x50
```

Squares signed `lh` deltas (`a0+0x268`/`+0x26C` minus `a1+0x2A`/`+0x32`)
and `jal func_8005186C(dx*dx + dz*dz)` with the sum in the delay slot.

The leftover residual under aspsx 2.21 was one missing `nop` between the
second `subu $v0,$v0,$v1` and `mult $v0,$v0`. That gap is not a load
delay (the first square is `subu`/`mult` with no nop). It is the ASPSX
≥ 2.30 `mflo`/`mult` two-instruction rule: after `mflo $a2` of the first
square, `subu` is only one instruction, so maspsx inserts `nop` before
the second `mult`. `--aspsx-version=2.21` sets `nop_mflo_mfhi=False` and
omits it. Per-leaf `ERA_ASPSX_VER=2.30` enables the gap; this leaf has
no `$at` expansions, so the other 2.30 knobs do not fire.

Unlinked object is reloc-only (`jal func_8005186C`). Linked
`0x80030534..0x80030584` is byte-identical to retail.

Mid-20210 carve: prefix `0xB24` remains `20210.s`, C `0x50`, resume
`20D84.s` `0xBC` (30584+305C8), then existing `func_80030640`.

## Files

```text
src/func_80030534.c
configs/USA/disc1.yaml          [0x20D34, c, func_80030534]
                                [0x20D84, asm]
```

## Commands

```text
scripts/split_us.sh
PATH=/tmp/pe-mipsel-wrap:$PATH scripts/build_us.sh
scripts/verify_us.sh
```
