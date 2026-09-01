# func_800CE1F4 — volume attempt 6

Outcome: `MATCHED@era -O2 -G0`, 2/2 words; leaf 287.

## Function hood and screens

- Span file `[0xBE9F4,0xBE9FC)`, VRAM `[0x800CE1F4,0x800CE1FC)`.
- Retail is `jr ra; move v0,zero`.
- Exact-start callback-table entry file `0xD17E0` (VRAM `0x800E0FE0`)
  contains `0x800CE1F4`.
- Real boundaries: preceding `move v0,zero` at `0x800CE1F0`; following
  prologue `addiu sp,sp,-24` at `0x800CE1FC`.

`FUNCTION_HOOD=PROVEN`. Frame 0; no calls, written globals, retained addresses,
indexed symbols, loops, repeated constants, or coloring pressure. `$v0` is
only the return. Flags: era `-O2 -G0`; no optional gate applies.

## Source, object, and carve

```c
int func_800CE1F4(void) {
    return 0;
}
```

```text
00000000 <func_800CE1F4>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero
```

The existing trim removes zero alignment padding. Prior asm
`[0xBE9F4,0xBEBAC) = 0x1B8` closes as:

```text
C 0xBE9FC-0xBE9F4 = 0x008
resume 0xBEBAC-0xBE9FC = 0x1B0
0x008 + 0x1B0 = 0x1B8
```

## Final gates

```text
probe file 0xBE9F4 (CE1F4): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
```

Packed candidate and ROM were identical (`cmp` exit 0):

```text
800ce1f0: 00001021  move v0,zero       # preceding
800ce1f4: 03e00008  jr ra              # leaf 1
800ce1f8: 00001021  move v0,zero       # leaf 2
800ce1fc: 27bdffe8  addiu sp,sp,-24    # following
```

`scripts/verify_us.sh` exited 0, reported exact SHA-1, and reported 287 leaves.
