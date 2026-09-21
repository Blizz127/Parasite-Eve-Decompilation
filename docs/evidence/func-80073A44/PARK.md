# func_80073A44 — PARKED (prologue-schedule + temp-spill residual)

**Status:** not C-matchable with the current toolchain. Not registered in
`configs/USA/disc1.yaml`. No `src/func_80073A44.c` is left in the tree.

## Target

- VRAM `0x80073A44`, file `0x64244`, span `0x178` (94 words).
- On-path fan-in **17** — the second-highest remaining asm leaf, called by the
  boot tail (`func_8001220C` calls it) and by `func_8006A5BC` / `func_800811E4`
  as the VSync read.

## What it is

A VSync/timer read with a cached previous value, plus two settle loops. Reads
two pointer globals and a word global:

| symbol | role |
|---|---|
| `D_80094574` | pointer; `*D_80094574` is the current status word |
| `D_80094578` | pointer; `*D_80094578` is the free-running counter |
| `D_8009457C` | last counter value (written at the end) |
| `D_80094580` | last status value (written mid-function) |
| `D_800956AC` | the "now" word returned for `a0 < 0` |
| `func_80073BBC(a, b)` | the settle/delay call |

Semantics: `s1 = (*D_80094578 - D_8009457C) & 0xFFFF`; dispatch on `a0`
(`<0` → return `D_800956AC`; `==1` → return `s1`; `<=0` → `D_80094580`; else
`D_80094580 - 1 + a0`), call `func_80073BBC(v, a0>0 ? a0-1 : 0)`, then
`func_80073BBC(D_800956AC + 1, 1)`; if the status word has bit `0x400000`, spin
until `(s0 ^ *D_80094574)` has bit `0x80000000` set; store `D_80094580 =
D_800956AC`; re-seed `D_8009457C` from `*D_80094578` with a stability loop;
return `s1`.

## Durable lever found (reusable, independent of this park)

**Pointer-to-volatile defeats the read-loop CSE.** With a plain
`extern unsigned int *`, cc1 CSEs the two reads in

```c
do { t = *D_80094578; } while (t != *D_80094578);
```

down to a single load (`lw $2,0($2)` reused) instead of retail's two
independent loads. Declaring the pointer

```c
extern volatile unsigned int *D_80094578;
```

reproduces retail's shape exactly:

```
$L2:
lw     $5,0($3)
lw     $2,0($3)
nop
bne    $5,$2,$L2
```

This is the same class of lever used by `func_8006599C`
(`extern unsigned char *volatile`) — pointer-to-volatile, applied to the
*pointee* here. The tail settle loop (re-seed `D_8009457C`, then compare
`D_8009457C != *D_80094578`) is reproduced correctly too.

`mult`/`multu` note: not applicable here; the only multiplication-free residual.

## The residual (why it cannot be closed)

With the read loop correct, the mismatch is **80 words** and is entirely
prologue scheduling, stack-spill and register allocation — no semantic
divergence:

1. **Pointer loads are hoisted above the prologue in retail.**
   Retail opens:
   ```
   64244: lui   $v0,%hi(D_80094574)
   64248: lw    $v0,%lo(D_80094574)($v0)
   6424C: lui   $a1,%hi(D_80094578)
   64250: lw    $a1,%lo(D_80094578)($a1)
   64254: addiu $sp,$sp,-0x28          <- prologue AFTER the loads
   64258: sw    $ra,0x20($sp)
   ...
   64264: lw    $s0,0x0($v0)
   ```
   cc1 always emits the prologue first; it will not schedule the two pointer
   loads into the region before `addiu $sp`. Same class as the
   `func_800701B4` callee-saved-init-scheduling park.

2. **Retail spills the loop temp to the stack.** Retail's read loop is 8 words:
   ```
   64268: lw    $v0,0x0($a1)
   6426C: nop
   64270: sw    $v0,0x10($sp)     <- spill t
   64274: lw    $v1,0x10($sp)     <- reload in the condition
   64278: lw    $v0,0x0($a1)
   6427C: nop
   64280: bne   $v1,$v0,.L80073A68
   ```
   cc1 keeps `t` live in a register across the loop (5 words), so the frame is
   `0x20` instead of retail's `0x28` (retail's slot at `0x10` is the spill slot
   above the three saved registers at `0x18/0x1C/0x20`).

3. **Register allocation:** retail assigns the saved counter to `$s1` and the
   status to `$s0`; cc1 picks `$16`/`$17` for the same roles but with a
   different pointer/scratch split (`$v0`/`$v1`/`$a1` vs `$2`/`$3`/`$5`).

## Levers tried (all fail)

- **Flag rungs:** `-O2 -G0` (80 mismatches); `-O1 -G0`, `-O0`, and
  `-fschedule-insns2` do not change the class.
- **Read-loop typing:** plain pointer (CSE'd, wrong shape), pointer-to-volatile
  (`volatile unsigned int *`, correct loop), volatile pointer
  (`unsigned int *volatile`, correct loop but double pointer reloads).
- **Expression/ordering forms:** the `a0` dispatch ladder written as
  `if/else-if` and as nested `if`; the settle loop written as `do/while` and as
  `for(;;)`; `v` computed via a ternary vs a branch — all reproduce the same
  logic with the same prologue/allocation residual.

## Would resolve it

A prologue-scheduling control (allow hoisting loads above the frame setup) or a
forced-spill lever for the loop temp — i.e. a `maspsx`/cc1-level change, not a C
spelling. Neither exists today.

## Evidence

- Retail disassembly: `asm/disc1/621E4.s:2518+` (`glabel func_80073A44` ..
  `endlabel`), 94 words.
- Link check: `tools/analysis/check_leaf.sh func_80073A44 0x80073A44 0x178 -O2 -G0`
  → `word mismatches=80` (with the pointer-to-volatile form; 74 with the plain
  pointer, different wrong shape).
