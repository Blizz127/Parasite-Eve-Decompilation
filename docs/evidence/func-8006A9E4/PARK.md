# func_8006A9E4 — PARKED (dbr-sched + copy-loop encoding residual)

**Status:** not C-matchable with the current toolchain. Not registered as a
`c` span. Draft remains `src/func_8006A9E4.c` with disposition
`in-progress-checkpoint` in `configs/USA/disc1_nonmatching_sources.json`
(same residual class as `func_800698D4`; not added to the 24-leaf
`ACCEPTED-RESIDUAL` policy list in this increment).

## Target

- VRAM `0x8006A9E4`, file `0x5B1E4`, span `0x35C` (215 words), frame `0x30`.
- Direct `jal` from `func_8001220C` at `0x80012284` (boot-spine image read).

## Residual mechanism

Best checkpoint (draft d10) is 216 words (+1). Frame, zone/copy *structure*,
and word counts of the copy loops are exact. Two source-invariant leftovers:

1. **Zone-1 retry back-edge delay slot** — retail leaves a `nop`; cc1 2.7.2
   steals the gate `li $v0,-1`. Same class as `func_800698D4` (dbr delay-slot
   liveness). Sentinel variables fix the steal and then ripple zone 2
   (register-pressure gates-vs-copies trade-off; d10 vs d14 bracket).
2. **Copy-loop addressing** — cc1 strength-reduces to `lw $v0,-8($a0)` vs
   retail's clean `lw $v1,4($a2)` with four consecutive loads. Same word
   count, different encodings; a temp-batched attempt added load-delay nops
   (+9).

Neither is source-expressible under the bounded phrasing/flag process.
Unblockers: a reorg/liveness lever (same as 698D4) and a copy-loop
addressing-mode lever.

## Evidence

`src/func_8006A9E4.c` header (d10 checkpoint, 2026-08-18). ROM
`asm/disc1/5B1E4.s`.
