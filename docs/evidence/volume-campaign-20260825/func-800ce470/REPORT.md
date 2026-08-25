# `func_800CE470` — exact signed-byte callback

Leaf 322, matched on the first phrasing.

## Function hood and boundaries

Retail span `[0xBEC70,0xBEC9C)`, VRAM `[0x800CE470,0x800CE49C)`, is
`0x2C` bytes / eleven words and ends in canonical `jr ra; nop`. Its exact
start occurs in the callback table at file `0xD17D0`, VRAM `0x800E0FD0`:

```text
0xD17CC: 800CE464
0xD17D0: 800CE470
0xD17D4: FFFFFFFF
```

The preceding real `func_800CE464` occupies `[0xBEC64,0xBEC70)` and ends
with `jr ra` plus its `sb` delay slot. The following real
`func_800CE49C` begins exactly at `0xBEC9C` with `addiu sp,sp,-0x18`.
Both boundary sides are executable instructions; there is no padding in the
candidate span.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_TABLE_ENTRY`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; callback leaf |
| written-state Stage 0 | argument-relative only: increments signed byte `counter[3]`; on value 6 writes byte 2 to `state[1]`; no global writer |
| coloring pressure | `$a1` retains `state`, `$a2` retains `counter`; `$v0` carries the loaded/incremented/re-sign-extended byte and then constant 2; `$v1` holds comparison constant 6 |
| `$v0` liveness | load → increment → store → signed-byte re-extension → compare; after the branch it is safely reused for constant 2 |
| address retention | none; both stores are fixed offsets from retained argument registers |
| optimization signal | compact load/increment/store sequence and scheduled `li v0,2` branch delay slot match era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

There are no callees, relocations, symbolic addresses, or global provenance
questions in this leaf.

## C and flags

```c
void func_800CE470(void *unused, signed char *state, signed char *counter) {
    if (++counter[3] == 6) {
        state[1] = 2;
    }
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; first phrasing, with no pins,
inline assembly, or special maspsx switch. Signed byte types are required by
retail's `sll 24; sra 24` after the store.

## Full eleven-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `90C20003` | `90C20003` | `lbu v0,3(a2)` |
| 1 | `24030006` | `24030006` | `li v1,6` |
| 2 | `24420001` | `24420001` | `addiu v0,v0,1` |
| 3 | `A0C20003` | `A0C20003` | `sb v0,3(a2)` |
| 4 | `00021600` | `00021600` | `sll v0,v0,24` |
| 5 | `00021603` | `00021603` | `sra v0,v0,24` |
| 6 | `14430002` | `14430002` | `bne v0,v1,+2` |
| 7 | `24020002` | `24020002` | `li v0,2` |
| 8 | `A0A20001` | `A0A20001` | `sb v0,1(a1)` |
| 9 | `03E00008` | `03E00008` | `jr ra` |
| 10 | `00000000` | `00000000` | `nop` |

Single-leaf object (there are no relocations to normalize):

```text
00000000 <func_800CE470>:
   0: 90c20003  lbu    v0,3(a2)
   4: 24030006  li     v1,6
   8: 24420001  addiu  v0,v0,1
   c: a0c20003  sb     v0,3(a2)
  10: 00021600  sll    v0,v0,0x18
  14: 00021603  sra    v0,v0,0x18
  18: 14430002  bne    v0,v1,0x24
  1c: 24020002  li     v0,2
  20: a0a20001  sb     v0,1(a1)
  24: 03e00008  jr     ra
  28: 00000000  nop
```

The compiler object's zero alignment word at `0x2C` is outside the
boundary-derived body and is rejected unless zero by the trim guard.

## Carve geometry

The prior asm span was `[0xBEC70,0xC5050)`, size `0x63E0`:

```text
prefix asm:  0xBEC70 - 0xBEC70 = 0x0000
C leaf:      0xBEC9C - 0xBEC70 = 0x002C
resume asm:  0xC5050 - 0xBEC9C = 0x63B4
closure:     0x0000 + 0x002C + 0x63B4 = 0x63E0
```

All sizes come from span-boundary arithmetic, never aligned object sizes.

## Packed-span and boundary proof

Retail and the packed candidate disassemble identically:

```text
800CE464: 24020002  li     v0,2
800CE468: 03E00008  jr     ra
800CE46C: A0A20001  sb     v0,1(a1)
800CE470: 90C20003  lbu    v0,3(a2)
800CE474: 24030006  li     v1,6
800CE478: 24420001  addiu  v0,v0,1
800CE47C: A0C20003  sb     v0,3(a2)
800CE480: 00021600  sll    v0,v0,24
800CE484: 00021603  sra    v0,v0,24
800CE488: 14430002  bne    v0,v1,0x800CE494
800CE48C: 24020002  li     v0,2
800CE490: A0A20001  sb     v0,1(a1)
800CE494: 03E00008  jr     ra
800CE498: 00000000  nop
800CE49C: 27BDFFE8  addiu  sp,sp,-24
800CE4A0: 00051080  sll    v0,a1,2
800CE4A4: AFBF0010  sw     ra,16(sp)
800CE4A8: 3C01800E  lui    at,0x800E
```

```text
retail:    00000000020002240800e0030100a2a00300c29006000324010042240300c2a0001602000316020002004314020002240100a2a00800e00300000000e8ffbd27801005001000bfaf0e80013c
candidate: 00000000020002240800e0030100a2a00300c29006000324010042240300c2a0001602000316020002004314020002240100a2a00800e00300000000e8ffbd27801005001000bfaf0e80013c
```

The build's packed leaf probe also reports identical 44-byte bodies:

```text
0300c29006000324010042240300c2a0001602000316020002004314020002240100a2a00800e00300000000
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
322
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 322 leaves
```

Result: `MATCHED=11/11`, first phrasing; consecutive-park count remains zero.
