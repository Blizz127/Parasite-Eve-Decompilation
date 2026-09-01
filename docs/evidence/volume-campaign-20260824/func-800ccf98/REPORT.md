# func_800CCF98 — volume attempt 4

Outcome: `MATCHED@era -O2 -G0`, 2/2 words; leaf 285.

## Function hood and screens

- File `[0xBD798,0xBD7A0)`, VRAM `[0x800CCF98,0x800CCFA0)`.
- Retail body: `jr ra; move v0,zero`.
- Exact-start callback-table entry at file `0xD169C` (VRAM
  `0x800E0E9C`) contains `0x800CCF98`.
- Real boundary words: preceding `move v0,zero` at `0x800CCF94`; following
  prologue `addiu sp,sp,-24` at `0x800CCFA0`.

`FUNCTION_HOOD=PROVEN`.

Frame is 0 (no args area, locals, or saves). There are no callees, written
globals, indexed symbols, retained addresses, loops, repeated constants, or
coloring pressure. `$v0` is live only as the zero return. Era `-O2 -G0` is
therefore the un-gated retail-consistent choice.

## Source and object

```c
int func_800CCF98(void) {
    return 0;
}
```

```text
00000000 <func_800CCF98>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero
```

The active 0x8 bytes match retail; the existing trim removes the object's
zero alignment tail.

## Carve and final gates

Prior asm `[0xBD798,0xBDADC) = 0x344`:

```text
C:       0xBD7A0 - 0xBD798 = 0x008
resume:  0xBDADC - 0xBD7A0 = 0x33C
closure: 0x008 + 0x33C = 0x344
```

Build probe and SHA:

```text
probe file 0xBD798 (CCF98): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
```

Packed span was identical (`cmp` exit 0):

```text
800ccf94: 00001021  move v0,zero       # preceding
800ccf98: 03e00008  jr ra              # leaf 1
800ccf9c: 00001021  move v0,zero       # leaf 2
800ccfa0: 27bdffe8  addiu sp,sp,-24    # following
```

`scripts/verify_us.sh` exited 0, reported exact SHA-1, and reported 285 leaves.
