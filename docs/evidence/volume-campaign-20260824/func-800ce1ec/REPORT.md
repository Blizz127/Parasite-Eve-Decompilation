# func_800CE1EC — volume attempt 5

Outcome: `MATCHED@era -O2 -G0`, 2/2 words; leaf 286.

## Function hood and screens

- Span file `[0xBE9EC,0xBE9F4)`, VRAM `[0x800CE1EC,0x800CE1F4)`.
- Retail is `jr ra; move v0,zero`.
- Exact-start callback-table entry file `0xD17F8` (VRAM `0x800E0FF8`)
  contains `0x800CE1EC`.
- Real boundaries: preceding `move v0,zero` at `0x800CE1E8`; following
  `jr ra` at `0x800CE1F4`.

`FUNCTION_HOOD=PROVEN`. Frame 0; no calls, written globals, address retention,
indexed symbols, loops, repeated constants, or coloring pressure. `$v0` is
only the return. Flags: era `-O2 -G0`; no optional gate applies.

## Source, object, and carve

```c
int func_800CE1EC(void) {
    return 0;
}
```

```text
00000000 <func_800CE1EC>:
   0: 03e00008  jr ra
   4: 00001021  move v0,zero
```

The existing trim removes the zero alignment tail. Prior asm
`[0xBE9EC,0xBEBAC) = 0x1C0` closes as:

```text
C 0xBE9F4-0xBE9EC = 0x008
resume 0xBEBAC-0xBE9F4 = 0x1B8
0x008 + 0x1B8 = 0x1C0
```

## Final gates

```text
probe file 0xBE9EC (CE1EC): cand=0800e00321100000 orig=0800e00321100000
RESULT: EXACT MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
```

Packed candidate and ROM were identical (`cmp` exit 0):

```text
800ce1e8: 00001021  move v0,zero       # preceding
800ce1ec: 03e00008  jr ra              # leaf 1
800ce1f0: 00001021  move v0,zero       # leaf 2
800ce1f4: 03e00008  jr ra              # following
```

`scripts/verify_us.sh` exited 0, reported exact SHA-1, and reported 286 leaves.
