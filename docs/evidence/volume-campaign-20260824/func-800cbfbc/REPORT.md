# func_800CBFBC — volume attempt 2

Outcome: `MATCHED@era -O2 -G0`, 2/2 words. Integrated as leaf 283.

## C1 / pool row

`0xBC7BC | func_800CBFBC | 2 words | jr-ra | 0 direct jal callers / 1 exact-start reference | 0 jal | no gp | no indexed symbolic access | no loop | no repeated constant | real/real boundaries | TIER 1`

Cycle-start tip was `08b07dd`; the tracked tree was clean.

## C2 / function hood

- Span: file `[0xBC7BC, 0xBC7C4)`, VRAM `[0x800CBFBC, 0x800CBFC4)`, two words.
- Body: `jr ra` with `move v0,zero` in the delay slot.
- Exact-start reference: the retail callback table entry at file `0xD1550`
  (VRAM `0x800E0D50`) contains `0x800CBFBC`.
- Previous boundary word at `0x800CBFB8` is the real instruction
  `move v0,zero` (the preceding callback's return delay slot).
- Following boundary word at `0x800CBFC4` is the real prologue instruction
  `addiu sp,sp,-0x20`.

`FUNCTION_HOOD=PROVEN`.

## C3 / screens and frame

| Screen | Result |
|---|---|
| Frame decomposition | args 0 + locals 0 + saves 0 = frame 0 |
| Callee buckets | none; no `jal` |
| Stage-0 written globals | none |
| Coloring pressure | none; only return register `$v0` |
| `$v0` liveness | return-zero value only; no branch/call interaction |
| Address retention | none |
| `-O` signal | none; no repeated materialization |
| Indexed symbolic gate | none |
| Loop/back-edge owner | none |

Flags: era `-O2 -G0`. `-G0` is justified by the absence of `$gp`; no 3W,
dispatch-fold, division, or sched2 gate applies.

## C4 / source

```c
int func_800CBFBC(void) {
    return 0;
}
```

## C6 / single-leaf object comparison

```text
00000000 <func_800CBFBC>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero

ROM  .text 8 bytes  C .text 16 bytes  target 8
SIZE_MISMATCH C=0x10 ROM=0x8
BYTE_EXACT (ignoring gas align pad)
```

The extra eight bytes are zero section-alignment padding and are removed by
the existing trim step; the active two-word body is exact.

## C7 / carve and full gates

Prior residual asm span: `[0xBC7BC, 0xBD780) = 0xFC4`.

```text
C leaf:       0xBC7C4 - 0xBC7BC = 0x008
asm resume:   0xBD780 - 0xBC7C4 = 0xFBC
closure:      0x008 + 0xFBC = 0xFC4
```

Full build tail:

```text
probe file 0xBC7BC (CBFBC): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

Packed boundary-span disassembly, candidate and ROM (`cmp` exit 0):

```text
800cbfb8: 00001021  move v0,zero       # preceding boundary
800cbfbc: 03e00008  jr ra              # leaf word 1
800cbfc0: 00001021  move v0,zero       # leaf word 2
800cbfc4: 27bdffe0  addiu sp,sp,-32    # following boundary
```

`scripts/verify_us.sh` exited 0 and reported:

```text
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 283 leaves
```
