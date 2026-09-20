# func_800D401C closer attempt — loop form improved, register allocation remains

Target: `func_800D401C` (file 0xC481C, 0x120, no direct calls) — 8-slot
`D_800E2368` allocator (stride 0xC from +0x20, `0xFFFF` = free) plus an
indirect `jalr` via `[s0+0x80][a0]`. Authority: retail Disc1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `asm/disc1/BF0F0.s` (function body
disassembled directly).

## Result

**No match.** Best candidate: **43/72 words differ** (down from the prior
agent's 63/72). The remaining differences are register-allocation/operand-order
only (no semantic differences), concentrated in the post-loop block.

## What improved it (a real lever)

The prior candidate used a `while` loop:
```c
while (*s1 != 0xFFFF) { a1++; if (a1 >= 8) break; s1 += 0xC; }
```
cc1 **rotated/peeled** that (63 differ). Rewriting it as an **inverted `for`
loop**:
```c
for (a1 = 0; a1 < 8; a1++) { if (*s1 == 0xFFFF) break; s1 += 0xC; }
if (a1 == 8) return;
```
produced retail's test-at-top loop shape and dropped the diff to **44**; moving
`sll` before the table load dropped it to 43. The `for` form is the recorded
loop-shape lever for this function.

## Remaining first mismatches (candidate v4)

The v1-vs-v0 role of the shifted index and table base, then the epilogue:
```
0x004C: retail 10A2002D (beq a1,v0,+0x2D)  cand 10A2002B
0x006C: retail 8E030080 (lw v1,128(s0))    cand 8E040080 (lw a0,...)
0x0070: retail 00041040 (sll v0,a0,1)      cand 8E050004
0x0074: retail 00431021 (addu v0,v0,v1)    cand 00641821
0x0078: retail 94520020 (lhu s2,32(v0))    cand 94640020
...
0x00CC: retail 3C01800F (lui at,D_800F33E0) cand 0040F809 (jalr)
```
Retail computes the table base in `v1` and the shifted index in `v0`; cc1
assigns `a0`/`v1` instead. Every expression order / cast / declaration order
tried (including splitting `v1 = a0<<1` before the base load) leaves the same
permutation. No `-O1`/`-fno-strength-reduce` rung helped.

## Commands

```
distrobox enter pe-mipsel -- bash -lc 'cd /home/blizz/dev/Parasite-Eve-Decompilation && python3 tools/analysis/try_leaf.py /tmp/candD401C_v4.c 0xC481C 0x120'
```

Best candidate parked at `nonmatch/func_800D401C.c`. No `src/`/`configs` change;
matching build untouched (708 leaves, EXACT SHA-1). No matching claim.

## Next lever to try

Give `s0` a **struct-typed** view (e.g. a 0xC-stride slot struct plus the
`+0x80` table) so cc1's address-mode/register choices match retail's
`v1`-base/`v0`-index split; or find a source order that makes the `beq a1,v0`
tail 2 words longer (the retail +0x2D vs candidate +0x2B branch displacement).
