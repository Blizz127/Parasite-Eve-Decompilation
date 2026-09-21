# func_80069B08 — PARKED (prologue save-batching + jumptable $at form)

**Status:** not C-matchable with the current toolchain. Not a YAML `c` span.
Draft `src/func_80069B08.c` is `in-progress-checkpoint`.

## Target

- VRAM `0x80069B08`, file `0x5A308`, size `0x5E0` (376 words), frame `0x58`.
- Direct `jal` from `func_8001220C` at `0x800122A8`.
- Jump table `jtbl_80011388` in prefix rodata (10 entries, cases 0..9).
- One `jr $v0` is the table dispatch, not a second function; terminal `jr $ra`
  at `0x8006A0E0`. Next glabel `func_8006A0E8`.

## What already matches

`era_link_check.py src/func_80069B08.c 0x80069B08 0x5E0 -O2 -G0` (no maspsx
patch): **linked `.text` 1504 bytes = declared `0x5E0`**, stack frame `0x58`,
`vars=32`, 9 saved regs. Locals packed as one struct so RECT is `0x10($sp)`,
`func_8007F418` dest `0x18($sp)`, `t20` `0x20($sp)`. The two
`func_8006E6A8`/`func_8006E7E8` CD-read/poll loops match retail's
`lw`/`lhu`/`addu`/`jal`/`subu`/`beq` shape.

## Residual mechanism

1. **Prologue save-batching (11 words, `0x80069B0C..0x80069B34`).** Retail
   interleaves `sw $s4` / `move $s4,$a0` then the zero inits of `$s2/$s6/$s7/$s1`.
   cc1 2.7.2 emits the constant inits first and copies `$a0` into `$s4` late.
   An empty `asm volatile` barrier on `$s4` batches *all* saves then inits
   (worse). Same class as `func_8001220C`.
2. **Jump-table addressing-mode coupling.** Retail's
   `sltiu` / `beqz` / `sll` / `lui $at,%hi(jtbl_80011388)` / `addu $at,$idx` /
   `lw $v0,%lo($at)` / `nop` / `jr $v0` is maspsx patch 5
   (`MASPSX_SYMBOL_AT_TEMP=1`). Turning the patch on also rewrites the
   `D_800B0E38[CDDC]` indexed loads from 4-word to 3-word, shrinking the
   leaf by 16 bytes (1488 vs 1504) and throwing every later branch offset.
   Without the patch the table load is the 4-word `lui`/`addiu %lo`/`addu`/`lw 0`
   form. No per-symbol knob selects 3-word for `jtbl_80011388` only.

Unblockers: a per-symbol `MASPSX_SYMBOL_AT_TEMP` list, or a prologue
save-interleave lever that copies `$a0` before constant s-reg inits.

## Evidence

Draft `src/func_80069B08.c`. ROM `asm/disc1/58518.s:2150-2563`.
`jtbl_80011388` at EXE file `0x1B88`.
