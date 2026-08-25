# `func_800905EC` — exact stream-byte flag helper

Leaf 314, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x80DEC,0x80E14)`, VRAM `0x800905EC`, is ten words and ends
in canonical `jr ra` plus a halfword-store delay slot. The exact-start
callback-table witness is:

```text
file 0x8D3B4 / VA 0x8009CBB4: .word func_800905EC
```

The preceding real `func_800905C4` ends at VA `0x800905E4/0x800905E8`
(`0x80DE4/0x80DE8` file offsets); the following real `func_80090614` starts
at file `0x80E14` with `lw v1,0(a0)`. Ownership is exact, with no padding at
either edge.

The pool's former `0/0 no caller/ref` screen was stale: it omitted the exact
callback-table word above. The function-hood requirement was re-proven before
the C attempt, and the row is removed from Tier 3 rather than Tier 1.

`FUNCTION_HOOD=PROVEN_BY_CALLBACK_TABLE_REFERENCE`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; stream-byte consumer/state mutator |
| written-global Stage 0 | no globals; argument-relative writes at `+0`, `+0xF4`, `+0x114` |
| coloring pressure | old cursor retained in `$v1`; increment and flag word use `$v0` |
| `$v0` liveness | no return value; cursor increment then flag read/modify/write |
| address retention | old cursor remains in `$v1` through cursor update and byte load |
| optimization signal | load-delay nop, useful `lbu` in a later load delay, final `sh` in return slot; exact GCC 14 `-O1` family |
| loop/back-edge owner | none |

## C and flags

```c
void func_800905EC(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x2200u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned short *)((unsigned char *)arg0 + 0x114) = byte;
}
```

GCC 14.2 production `CFLAGS_LEAF`:

```text
-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls
-ffreestanding -fno-builtin -O1
```

The flags and source form come from independently verified siblings
`func_8009059C`, `func_800905C4`, and `func_800906B4`. No pins, assembly,
maspsx switch, or relocation normalization is involved.

## Full ten-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `8C830000` | `8C830000` | `lw v1,0(a0)` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `24620001` | `24620001` | `addiu v0,v1,1` |
| 3 | `AC820000` | `AC820000` | `sw v0,0(a0)` |
| 4 | `8C8200F4` | `8C8200F4` | `lw v0,0xF4(a0)` |
| 5 | `90630000` | `90630000` | `lbu v1,0(v1)` |
| 6 | `34422200` | `34422200` | `ori v0,v0,0x2200` |
| 7 | `AC8200F4` | `AC8200F4` | `sw v0,0xF4(a0)` |
| 8 | `03E00008` | `03E00008` | `jr ra` |
| 9 | `A4830114` | `A4830114` | `sh v1,0x114(a0)` |

Object disassembly:

```text
00000000 <func_800905EC>:
   0: 8c830000  lw     v1,0(a0)
   4: 00000000  nop
   8: 24620001  addiu  v0,v1,1
   c: ac820000  sw     v0,0(a0)
  10: 8c8200f4  lw     v0,244(a0)
  14: 90630000  lbu    v1,0(v1)
  18: 34422200  ori    v0,v0,0x2200
  1c: ac8200f4  sw     v0,244(a0)
  20: 03e00008  jr     ra
  24: a4830114  sh     v1,276(a0)
```

## Carve and packed-span proof

The active asm span before this carve was `[0x80DEC,0x80EB4)`, size `0xC8`:

```text
prefix asm: 0x80DEC - 0x80DEC = 0x00
C leaf:     0x80E14 - 0x80DEC = 0x28
resume asm: 0x80EB4 - 0x80E14 = 0xA0
closure:    0x00 + 0x28 + 0xA0 = 0xC8
```

The packed candidate and retail disassemble identically across both real
function boundaries:

```text
800905e4: 03e00008  jr     ra
800905e8: a4830112  sh     v1,274(a0)
800905ec: 8c830000  lw     v1,0(a0)
800905f0: 00000000  nop
800905f4: 24620001  addiu  v0,v1,1
800905f8: ac820000  sw     v0,0(a0)
800905fc: 8c8200f4  lw     v0,244(a0)
80090600: 90630000  lbu    v1,0(v1)
80090604: 34422200  ori    v0,v0,0x2200
80090608: ac8200f4  sw     v0,244(a0)
8009060c: 03e00008  jr     ra
80090610: a4830114  sh     v1,276(a0)
80090614: 8c830000  lw     v1,0(a0)
80090618: 00000000  nop
```

```text
retail:    0000838c0000000001006224000082acf400828c0000639000224234f40082ac0800e003140183a4
candidate: 0000838c0000000001006224000082acf400828c0000639000224234f40082ac0800e003140183a4
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
314
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 314 leaves
```

Result: `MATCHED=10/10`, first phrasing; consecutive-park count resets to
zero.
