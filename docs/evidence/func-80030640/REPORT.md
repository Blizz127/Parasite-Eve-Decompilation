# func_80030640 — RNG gate matching C (40 words)

```text
MATCHING_C — func_80030640 era -O2 -G0, 40/40 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x80030640..0x800306E0 exclusive
file        0x20E40 size 0xA0
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 230
func_80030640.c.o .text: 0xA0→0xA0
```

If `*(D_8009D278+0x68)+0x10` has bit 16 set and `func_80071A54() % 100`
is signed-less than `lhu rec+0x22`, store 9000 at `rec+0x10`.

Proven vs this tree's `asm/disc1/20210.s` (pre-carve) and the linked
candidate:

- ROM `lui $v1,1` / `and` is `0x10000`. `& 1` emits `andi` (hard mismatch).
- Second `D_8009D278` load is `$v1`. The signed `%100` expansion clobbers
  `$a0`, so C reloads into a separate `rec2` instead of reusing `rec`.
- Non-leaf: frame `-0x18`, `$ra` + `$s0` (threshold).
- Unlinked object is reloc-only (`lui`/`lw`/`jal` immediates). Linked
  `0x80030640..0x800306E0` is byte-identical to retail.

Mid-20210 carve: prefix `0xC30` remains `20210.s`, C `0xA0`, resume
`20EE0.s` `0x3340`. `0x24220` / `func_80033A20` unchanged.

## Files

```text
src/func_80030640.c
configs/USA/disc1.yaml          [0x20E40, c, func_80030640]
                                [0x20EE0, asm]
```

## Commands

```text
scripts/split_us.sh
PATH=/tmp/pe-mipsel-wrap:$PATH scripts/build_us.sh
scripts/verify_us.sh
```
