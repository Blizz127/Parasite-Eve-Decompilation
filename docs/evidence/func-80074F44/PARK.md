# func_80074F44 — PARKED (prologue-interleave scheduling residual)

**Status:** not C-matchable with the current toolchain. Not registered in
`configs/USA/disc1.yaml`. No `src/func_80074F44.c` is left in the tree.

## Target

- VRAM `0x80074F44`, file `0x65744`, span `0x90` (36 words).
- On-path fan-in **6** — a `D_80095744` handler dispatch in the `0x80074xxx`
  boot-tail cluster (same global as the already-matched `func_80074D28` /
  `func_80074DC0`).

## Semantics (proven)

```c
func_80074E28(&D_800118BC, a0);                 /* seed a primitive */
h = D_80095744;
return ((int (*)())h[2])(h[3], a0, 8,
        ((a3 & 0xFF) << 16) | ((a2 & 0xFF) << 8) | (a1 & 0xFF));
```

`D_80095744` is the **pointer global** (`extern unsigned int *`) whose `+0x8`
handler and `+0xC` argument word this cluster reads through one loaded base.
`&D_800118BC` must be a **data-symbol argument** (an address literal folds to a
single `lui` and loses the `%lo`).

## The residual (why it cannot be closed)

Every non-prologue word matches: the call setup, the three `andi`/`sll`/`or`
pack and the `jalr` all reproduce. The remainder is **9 words, entirely
prologue/callee-saved interleave** — cc1 orders the save/argument-shuffle chain
differently from retail:

```
retail                          cc1
0x4F4C addu $s3,$a0,$zero       sw   $s2,0x18($sp)
0x4F50 lui  $a0,%hi(D_800118BC) addu $s2,$a1,$zero
0x4F54 addiu $a0,$a0,%lo        sw   $s1,0x14($sp)
0x4F58 sw   $s2,0x18($sp)       addu $s1,$a2,$zero
0x4F5C addu $s2,$a1,$zero       sw   $s0,0x10($sp)
0x4F60 addu $a1,$s3,$zero       addu $s0,$a3,$zero
0x4F64 sw   $s1,0x14($sp)       lui  $a0,%hi(D_800118BC)
0x4F68 addu $s1,$a2,$zero       addiu $a0,$a0,%lo
0x4F6C sw   $s0,0x10($sp)       addu $a1,$s3,$zero
0x4F78 addu $s0,$a3,$zero       sw   $ra,0x20($sp)
```

Retail materializes the `&D_800118BC` argument *between* saving `$s3` and
saving `$s2`; cc1 emits all four saves first, then the address. This is the same
class as the `func_80073A44` / `func_800701B4` prologue-scheduling parks.

`word mismatches = 9` with `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1` (patch 3), down
from 11 without it (patch 3 correctly fills the epilogue `addiu $sp` into the
`jr $ra` delay slot).

## Levers tried

| form | mismatches |
|---|---|
| address literal `(unsigned char *)0x800118BC` | (rejected — folds to one `lui`) |
| `&D_800118BC` data symbol, patch 3 off | 11 |
| `&D_800118BC` + **patch 3 on** | **9** |
| named local `p = &D_800118BC` | 11 |
| pinned `register unsigned char *p asm("$4")` | 9 |
| packed value computed first | 30 |
| `h = D_80095744` hoisted before the call | 33 |
| `-O1 -G0` / `-O1 -G0 -fschedule-insns2` / `-O2 -fno-schedule-insns` / `-O0` | 34 / 30 / 30 / 35 |

## Would resolve it

A prologue-scheduling control (interleave an argument address materialization
between the callee-saved stores) — a cc1/`maspsx` change, not a C spelling.

## Evidence

- Retail disassembly: `asm/disc1/65628.s` (`glabel func_80074F44` .. `endlabel`),
  36 words.
- Link check: `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1 python3
  tools/analysis/era_link_check.py src/func_80074F44.c 0x80074F44 0x90 -O2 -G0`
  → `word mismatches=9`.
