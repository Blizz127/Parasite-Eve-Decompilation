# func_800CBFB4 — volume attempt 1

Outcome: `MATCHED@era -O2 -G0`, 2/2 words. Integrated as leaf 282.

## C1 / pool row

`0xBC7B4 | func_800CBFB4 | 2 words | jr-ra | 0 direct jal callers / 1 exact-start reference | 0 jal | no gp | no indexed symbolic access | no loop | no repeated constant | real/real boundaries | TIER 1`

Cycle-start tip was `fb9b81b`; the tracked tree was clean.

## C2 / function hood

- Span: file `[0xBC7B4, 0xBC7BC)`, VRAM `[0x800CBFB4, 0x800CBFBC)`, two words.
- Body: `jr ra` with `move v0,zero` in the delay slot.
- Exact-start reference: the retail callback table entry at file `0xD1568`
  (VRAM `0x800E0D68`) is `.word func_800CBFB4`.
- Previous boundary word at `0x800CBFB0` is the real instruction
  `move v0,zero` (the preceding return's delay slot).
- Following boundary word at `0x800CBFBC` is the real instruction `jr ra`,
  the start of the next callback.

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
int func_800CBFB4(void) {
    return 0;
}
```

## C6 / single-leaf object comparison

```text
00000000 <func_800CBFB4>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero

ROM  .text 8 bytes  C .text 16 bytes  target 8
SIZE_MISMATCH C=0x10 ROM=0x8
BYTE_EXACT (ignoring gas align pad)
```

The extra eight bytes are zero section-alignment padding and are removed by
the existing trim step; the active two-word body is exact.

## C7 / carve and full gates

Prior asm span: `[0xBC7B4, 0xBD780) = 0xFCC`.

```text
C leaf:       0xBC7BC - 0xBC7B4 = 0x008
asm resume:   0xBD780 - 0xBC7BC = 0xFC4
closure:      0x008 + 0xFC4 = 0xFCC
```

Full build tail:

```text
orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
probe file 0xBC7B4 (CBFB4): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
```

Packed boundary-span disassembly (candidate and ROM were identical; `cmp`
reported no differences):

```text
800cbfb0: 00001021  move v0,zero       # preceding boundary
800cbfb4: 03e00008  jr ra              # leaf word 1
800cbfb8: 00001021  move v0,zero       # leaf word 2
800cbfbc: 03e00008  jr ra              # following boundary
```

`scripts/verify_us.sh` exited 0 and reported:

```text
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 282 leaves
```
