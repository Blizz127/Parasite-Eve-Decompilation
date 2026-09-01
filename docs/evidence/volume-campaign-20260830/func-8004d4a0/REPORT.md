# `func_8004D4A0` — exact fixed-allocation boolean wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 373 and Tier-2 continuation rung 32.

## Function hood

- File `[0x3DCA0,0x3DCC4)`, VA `[0x8004D4A0,0x8004D4C4)`: nine words,
  ending in canonical `jr ra` with stack teardown in the delay slot.
- Exact direct caller `0x8004083C` targets the start.
- Preceding real `func_8004D2DC` ends at `0x3DC98/0x3DC9C`; following real
  `func_8004D4C4` starts immediately at `0x3DCC4` with a nontrivial prologue.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## Retail, screens, and C

```text
3DCA0 8004D4A0 E8FFBD27  addiu sp,sp,-0x18
3DCA4 8004D4A4 1000BFAF  sw    ra,0x10(sp)
3DCA8 8004D4A8 01000424  addiu a0,zero,1
3DCAC 8004D4AC 8D8A010C  jal   func_80062A34
3DCB0 8004D4B0 24000524  addiu a1,zero,0x24
3DCB4 8004D4B4 1000BF8F  lw    ra,0x10(sp)
3DCB8 8004D4B8 2B100200  sltu  v0,zero,v0
3DCBC 8004D4BC 0800E003  jr    ra
3DCC0 8004D4C0 1800BD27  addiu sp,sp,0x18
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_80062A34` call with fixed arguments 1 and `0x24` |
| Stage-0 | no direct globals; effects belong to the allocator/callee |
| Coloring / `$v0` | callee result remains in `$v0` and is canonicalized in place |
| Address retention | none |
| `-O` signal | era `-O2` puts `0x24` in the call delay and stack teardown in return delay |
| Loop | none |

```c
int func_80062A34(int type, int size);

int func_8004D4A0(void) {
    return func_80062A34(1, 0x24) != 0;
}
```

The `!= 0` spelling is proven by retail `sltu v0,zero,v0`. Flags: era `-O2
-G0`; no maspsx gate.

## Object, carve, and packed gate

```text
object: 27bdffe8 afbf0010 24040001 0c000000 24050024 8fbf0010 0002102b 03e00008 27bd0018
ROM:    27bdffe8 afbf0010 24040001 0c018a8d 24050024 8fbf0010 0002102b 03e00008 27bd0018
C:      27bdffe8 afbf0010 24040001 0c018a8d 24050024 8fbf0010 0002102b 03e00008 27bd0018
RELOCS_NORMALIZED=one R_MIPS_26 func_80062A34
BYTE_EXACT=9/9
```

The standalone object has only zero alignment after the `0x24` body; guarded
trim accepts it. Former asm `[0x3DA98,0x3E29C)` = `0x0804`:

```text
prefix: 0x3DCA0 - 0x3DA98 = 0x0208
leaf:   0x3DCC4 - 0x3DCA0 = 0x0024
resume: 0x3E29C - 0x3DCC4 = 0x05D8
close:  0x0208 + 0x0024 + 0x05D8 = 0x0804
```

Packed candidate and retail:

```text
3DC90: 8fb00010 = 8fb00010  preceding restore
3DC94: 27bd0020 = 27bd0020  preceding teardown
3DC98: 03e00008 = 03e00008  preceding return
3DC9C: 00000000 = 00000000  preceding delay
3DCA0: 27bdffe8 = 27bdffe8  leaf 1
3DCA4: afbf0010 = afbf0010  leaf 2
3DCA8: 24040001 = 24040001  leaf 3
3DCAC: 0c018a8d = 0c018a8d  leaf 4
3DCB0: 24050024 = 24050024  leaf 5
3DCB4: 8fbf0010 = 8fbf0010  leaf 6
3DCB8: 0002102b = 0002102b  leaf 7
3DCBC: 03e00008 = 03e00008  leaf 8
3DCC0: 27bd0018 = 27bd0018  leaf 9
3DCC4: 27bdffd8 = 27bdffd8  following real prologue
3DCC8: afb3001c = afb3001c  following word 2
3DCCC: 00809821 = 00809821  following word 3
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 373
```
