# `func_80064E90` — exact nested-state clear wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 379 and Tier-2 continuation rung 38.

## Function hood and screens

- File `[0x55690,0x556B4)`, VA `[0x80064E90,0x80064EB4)`: nine words,
  ending in canonical `jr ra; nop`.
- Exact direct caller `0x80064924` targets the start.
- Preceding real `func_80064D08` ends at `0x55688/0x5568C`; following real
  `func_80064EB4` starts immediately at `0x556B4`.
- One unresolved callee, `func_8006269C`; no globals or loop. The field-34
  pointer remains in `$v0` only until its nested `+0x80` store in the call
  delay slot. The call result is dead and there is no result-register pressure.
- Typed offsets `0x34` and `0x80` are directly retail-proven. No symbolic
  addressing or constant-materialization signal requires a maspsx gate;
  established era `-O2 -G0` call scheduling is sufficient.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`.

## C and object

```c
typedef struct {
    unsigned char pad00[0x34];
    int *state;
} Func80064E90Object;

void func_8006269C(Func80064E90Object *object);

void func_80064E90(Func80064E90Object *object) {
    object->state[0x20] = 0;
    func_8006269C(object);
}
```

Flags: era `-O2 -G0`; no maspsx gate.

```text
55690: 27bdffe8  addiu sp,sp,-24
55694: afbf0010  sw    ra,16(sp)
55698: 8c820034  lw    v0,52(a0)
5569C: 0c0189a7  jal   func_8006269C
556A0: ac400080  sw    zero,128(v0)
556A4: 8fbf0010  lw    ra,16(sp)
556A8: 27bd0018  addiu sp,sp,24
556AC: 03e00008  jr    ra
556B0: 00000000  nop

object: 27bdffe8 afbf0010 8c820034 0c000000 ac400080 8fbf0010 27bd0018 03e00008 00000000
ROM/C:  27bdffe8 afbf0010 8c820034 0c0189a7 ac400080 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=R_MIPS_26 func_8006269C
BYTE_EXACT=9/9
```

## Carve and packed gate

Former asm `[0x55454,0x55BB8)` = `0x0764`:

```text
prefix: 0x55690 - 0x55454 = 0x023C
leaf:   0x556B4 - 0x55690 = 0x0024
resume: 0x55BB8 - 0x556B4 = 0x0504
close:  0x023C + 0x0024 + 0x0504 = 0x0764
```

```text
55680: 8fb00010 = 8fb00010  preceding epilogue
55684: 27bd0028 = 27bd0028
55688: 03e00008 = 03e00008
5568C: 00000000 = 00000000
55690: 27bdffe8 = 27bdffe8  leaf 1
55694: afbf0010 = afbf0010  leaf 2
55698: 8c820034 = 8c820034  leaf 3
5569C: 0c0189a7 = 0c0189a7  leaf 4
556A0: ac400080 = ac400080  leaf 5
556A4: 8fbf0010 = 8fbf0010  leaf 6
556A8: 27bd0018 = 27bd0018  leaf 7
556AC: 03e00008 = 03e00008  leaf 8
556B0: 00000000 = 00000000  leaf 9
556B4: 27bdffe0 = 27bdffe0  following real prologue
556B8: afb00010 = afb00010
556BC: 00808021 = 00808021
PACKED_SPAN=EXACT
```

```text
retail/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/build_us.sh: exit 0; Compare: EXACT SHA-1 MATCH
matching-C count: 379
```
