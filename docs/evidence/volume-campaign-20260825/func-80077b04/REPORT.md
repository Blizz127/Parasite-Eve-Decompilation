# `func_80077B04` — exact matching-C byte flag helper

Leaf 310, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x68304,0x6832C)`, VRAM `0x80077B04`, ten words. The body
ends in the canonical `jr ra` with its byte store in the return delay slot.
Five direct `jal` instructions target the exact start:

```text
0x80030A10
0x80030B18
0x80037404
0x800D1E80
0x800D291C
```

The neighboring functional boundaries are unambiguous. Preceding
`func_80077AC4` ends at `0x80077AFC` with the store in its `jr ra` delay
slot, followed by one alignment nop at file `0x68300`. This leaf ends at
file `0x68328`; two alignment nops at `0x6832C` and `0x68330` precede the
real first instruction of `func_80077B34` at `0x68334`. The alignment words
are outside the called leaf and are retained as asm.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; pure byte-field helper |
| written-global Stage 0 | none; only argument-relative byte `a0[7]` is written |
| coloring pressure | low; `$a0/$a1` remain inputs and `$v0` carries the loaded/result byte |
| `$v0` liveness | loaded separately in each branch, transformed, then stored in the return delay slot; no C return value |
| address retention | none; fixed argument-relative offset `7` throughout |
| optimization signal | duplicated branch-local `lbu`, load-delay nop on the clear path, and scheduled return-slot store select era `-O2` |
| loop/back-edge owner | none |

The `0x02` set mask and `0xFD` clear mask prove a set/clear operation on bit
1 of the byte at offset 7. No semantic structure name is assigned without
caller/type evidence.

## C and flags

```c
void func_80077B04(unsigned char *a0, int a1) {
    if (a1 != 0) {
        a0[7] |= 2;
    } else {
        a0[7] &= 0xFD;
    }
}
```

Compiled with `era_compile ... -O2 -G0`. There are no globals, pins, inline
assembly, or special maspsx switches. The object has one local
`R_MIPS_26 .text` relocation for the jump over the clear arm; after normal
link resolution it targets retail `0x80077B24`.

## Full single-leaf comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `10A00004` | `10A00004` | `beqz a1,+4` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `90820007` | `90820007` | `lbu v0,7(a0)` |
| 3 | `0801DEC9` | `0801DEC9` | `j 0x80077B24` (local relocation normalized) |
| 4 | `34420002` | `34420002` | `ori v0,v0,2` |
| 5 | `90820007` | `90820007` | `lbu v0,7(a0)` |
| 6 | `00000000` | `00000000` | `nop` |
| 7 | `304200FD` | `304200FD` | `andi v0,v0,0xFD` |
| 8 | `03E00008` | `03E00008` | `jr ra` |
| 9 | `A0820007` | `A0820007` | `sb v0,7(a0)` |

Object disassembly before link:

```text
00000000 <func_80077B04>:
   0: 10a00004  beqz  a1,14
   4: 00000000  nop
   8: 90820007  lbu    v0,7(a0)
   c: 08000008  j      20
                    c: R_MIPS_26 .text
  10: 34420002  ori    v0,v0,0x2
  14: 90820007  lbu    v0,7(a0)
  18: 00000000  nop
  1c: 304200fd  andi   v0,v0,0xfd
  20: 03e00008  jr     ra
  24: a0820007  sb     v0,7(a0)
```

## Carve and packed-span proof

The prior asm span was `[0x682BC,0x68364)`, size `0xA8`:

```text
prefix asm: 0x68304 - 0x682BC = 0x48
C leaf:     0x6832C - 0x68304 = 0x28
resume asm: 0x68364 - 0x6832C = 0x38
closure:    0x48 + 0x28 + 0x38 = 0xA8
```

These sizes come from file-boundary subtraction, not aligned object sizes.
Full packed span, file `0x68304..0x6832B`:

```text
retail:    0400a0100000000007008290c9de0108020042340700829000000000fd0042300800e003070082a0
candidate: 0400a0100000000007008290c9de0108020042340700829000000000fd0042300800e003070082a0
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh` exits
zero and reports 310 matching-C leaves.
