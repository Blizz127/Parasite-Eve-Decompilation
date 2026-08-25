# func_800CCF90 — volume attempt 3

Outcome: `MATCHED@era -O2 -G0`, 2/2 words. Integrated as leaf 284.

## C1 / pool row

`0xBD790 | func_800CCF90 | 2 words | jr-ra | 0 direct jal callers / 1 exact-start reference | 0 jal | no gp | no indexed symbolic access | no loop | no repeated constant | real/real boundaries | TIER 1`

Cycle-start tip was `c06c9d6`; the tracked tree was clean.

## C2 / function hood

- Span: file `[0xBD790, 0xBD798)`, VRAM `[0x800CCF90, 0x800CCF98)`, two words.
- Body: `jr ra` with `move v0,zero` in the delay slot.
- Exact-start reference: retail callback table entry file `0xD16B4`
  (VRAM `0x800E0EB4`) contains `0x800CCF90`.
- Previous boundary word `0x800CCF8C` is the real `move v0,zero` delay
  instruction of the preceding callback.
- Following boundary word `0x800CCF98` is the real `jr ra` opening the next
  callback.

`FUNCTION_HOOD=PROVEN`.

## C3 / screens and frame

| Screen | Result |
|---|---|
| Frame decomposition | args 0 + locals 0 + saves 0 = frame 0 |
| Callee buckets | none; no `jal` |
| Stage-0 written globals | none |
| Coloring pressure | none; only return register `$v0` |
| `$v0` liveness | return-zero value only |
| Address retention | none |
| `-O` signal | none; no materialized constants |
| Indexed symbolic gate | none |
| Loop/back-edge owner | none |

Flags: era `-O2 -G0`; no `$gp` or optional assembler/compiler gate is
indicated by retail.

## C4 / source

```c
int func_800CCF90(void) {
    return 0;
}
```

## C6 / single-leaf object comparison

```text
00000000 <func_800CCF90>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero
```

The two active words match retail. The object `.text` is 0x10 before the
existing alignment-pad trim and 0x8 after it.

## C7 / carve and full gates

Prior asm span: `[0xBD790, 0xBDADC) = 0x34C`.

```text
C leaf:       0xBD798 - 0xBD790 = 0x008
asm resume:   0xBDADC - 0xBD798 = 0x344
closure:      0x008 + 0x344 = 0x34C
```

Full build:

```text
probe file 0xBD790 (CCF90): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

Packed boundary span, identical candidate and ROM (`cmp` exit 0):

```text
800ccf8c: 00001021  move v0,zero       # preceding boundary
800ccf90: 03e00008  jr ra              # leaf word 1
800ccf94: 00001021  move v0,zero       # leaf word 2
800ccf98: 03e00008  jr ra              # following boundary
```

`scripts/verify_us.sh` exited 0 and reported exact SHA-1 and 284 leaves.
