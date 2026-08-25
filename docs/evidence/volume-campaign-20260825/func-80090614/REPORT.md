# `func_80090614` — exact stream-byte flag helper

Leaf 315, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x80E14,0x80E3C)`, VRAM `0x80090614`, is ten words and ends
in canonical `jr ra` plus a halfword-store delay slot. Its exact-start
callback-table witness is:

```text
file 0x8D3B8 / VA 0x8009CBB8: .word func_80090614
```

The preceding real `func_800905EC` ends at VA `0x8009060C/0x80090610`
(`0x80E0C/0x80E10` file offsets); the following real `func_8009063C` starts
at file `0x80E3C` with `lw v1,0(a0)`. There is no padding at either edge.

The pool's former `0/0 no caller/ref` screen omitted the callback-table word.
The function-hood requirement was re-proven before the C attempt, and this
match removes the row from Tier 3.

`FUNCTION_HOOD=PROVEN_BY_CALLBACK_TABLE_REFERENCE`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; stream-byte consumer/state mutator |
| written-global Stage 0 | no globals; argument-relative writes at `+0`, `+0xF4`, `+0x116` |
| coloring pressure | old cursor retained in `$v1`; increment and flag word use `$v0` |
| `$v0` liveness | no return value; cursor increment then flag read/modify/write |
| address retention | old cursor remains in `$v1` through cursor update and byte load |
| optimization signal | load-delay nop, useful `lbu` in a later load delay, final `sh` in return slot; exact GCC 14 `-O1` family |
| loop/back-edge owner | none |

## C and flags

```c
void func_80090614(void *arg0) {
    unsigned int v0;
    unsigned char *v1;
    unsigned char byte;

    v1 = *(unsigned char **)arg0;
    *(unsigned char **)arg0 = v1 + 1;
    v0 = *(unsigned int *)((unsigned char *)arg0 + 0xF4);
    byte = *v1;
    v0 |= 0x4400u;
    *(unsigned int *)((unsigned char *)arg0 + 0xF4) = v0;
    *(unsigned short *)((unsigned char *)arg0 + 0x116) = byte;
}
```

GCC 14.2 production `CFLAGS_LEAF`:

```text
-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls
-ffreestanding -fno-builtin -O1
```

The source and flags are independently verified by the adjacent helper
family. No pins, assembly, maspsx switch, or relocation normalization is
involved.

## Full ten-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `8C830000` | `8C830000` | `lw v1,0(a0)` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `24620001` | `24620001` | `addiu v0,v1,1` |
| 3 | `AC820000` | `AC820000` | `sw v0,0(a0)` |
| 4 | `8C8200F4` | `8C8200F4` | `lw v0,0xF4(a0)` |
| 5 | `90630000` | `90630000` | `lbu v1,0(v1)` |
| 6 | `34424400` | `34424400` | `ori v0,v0,0x4400` |
| 7 | `AC8200F4` | `AC8200F4` | `sw v0,0xF4(a0)` |
| 8 | `03E00008` | `03E00008` | `jr ra` |
| 9 | `A4830116` | `A4830116` | `sh v1,0x116(a0)` |

```text
00000000 <func_80090614>:
   0: 8c830000  lw     v1,0(a0)
   4: 00000000  nop
   8: 24620001  addiu  v0,v1,1
   c: ac820000  sw     v0,0(a0)
  10: 8c8200f4  lw     v0,244(a0)
  14: 90630000  lbu    v1,0(v1)
  18: 34424400  ori    v0,v0,0x4400
  1c: ac8200f4  sw     v0,244(a0)
  20: 03e00008  jr     ra
  24: a4830116  sh     v1,278(a0)
```

## Carve and packed-span proof

The active asm span was `[0x80E14,0x80EB4)`, size `0xA0`:

```text
prefix asm: 0x80E14 - 0x80E14 = 0x00
C leaf:     0x80E3C - 0x80E14 = 0x28
resume asm: 0x80EB4 - 0x80E3C = 0x78
closure:    0x00 + 0x28 + 0x78 = 0xA0
```

Packed candidate and retail disassemble identically across both real
function boundaries:

```text
8009060c: 03e00008  jr     ra
80090610: a4830114  sh     v1,276(a0)
80090614: 8c830000  lw     v1,0(a0)
80090618: 00000000  nop
8009061c: 24620001  addiu  v0,v1,1
80090620: ac820000  sw     v0,0(a0)
80090624: 8c8200f4  lw     v0,244(a0)
80090628: 90630000  lbu    v1,0(v1)
8009062c: 34424400  ori    v0,v0,0x4400
80090630: ac8200f4  sw     v0,244(a0)
80090634: 03e00008  jr     ra
80090638: a4830116  sh     v1,278(a0)
8009063c: 8c830000  lw     v1,0(a0)
80090640: 00000000  nop
```

```text
retail:    0000838c0000000001006224000082acf400828c0000639000444234f40082ac0800e003160183a4
candidate: 0000838c0000000001006224000082acf400828c0000639000444234f40082ac0800e003160183a4
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
315
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 315 leaves
```

Result: `MATCHED=10/10`, first phrasing; consecutive-park count remains zero.
