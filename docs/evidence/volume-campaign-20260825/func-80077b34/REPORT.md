# `func_80077B34` — exact matching-C byte flag helper

Leaf 311, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x68334,0x6835C)`, VRAM `0x80077B34`, ten words. It ends in
the canonical `jr ra` with the final byte store in the return delay slot.
Four direct `jal` instructions target the exact start:

```text
0x80030E0C
0x80030E70
0x80030ED0
0x80037340
```

The neighboring functional boundaries are explicit. Two alignment nops at
file `0x6832C` and `0x68330` separate this entry from the preceding called
leaf; two more at `0x6835C` and `0x68360` separate its return from the real
first instruction of `func_80077B64` at `0x68364`. Both padding fragments
remain asm and are outside the exact called span.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; pure byte-field helper |
| written-global Stage 0 | none; only argument-relative byte `a0[7]` is written |
| coloring pressure | low; `$v0` carries the loaded/result byte |
| `$v0` liveness | branch-local load, bit transform, then store in the return delay slot; no C return value |
| address retention | none; fixed argument-relative offset `7` |
| optimization signal | duplicated branch-local `lbu`, load-delay nop on the clear path, and return-slot store select era `-O2` |
| loop/back-edge owner | none |

The `0x01` set mask and `0xFE` clear mask prove a set/clear operation on bit
0 of the byte at offset 7. Existing B54I GPU-leaf evidence identifies the
Psy-Q role as `SetShadeTex`; this source retains the unresolved executable
symbol and minimal proven ABI.

## C and flags

```c
void func_80077B34(unsigned char *a0, int a1) {
    if (a1 != 0) {
        a0[7] |= 1;
    } else {
        a0[7] &= 0xFE;
    }
}
```

Compiled with `era_compile ... -O2 -G0`. No globals, pins, inline assembly,
or special maspsx switches are used. The object-local
`R_MIPS_26 .text` relocation resolves from `08000008` to retail
`0801DED5`, targeting `0x80077B54`.

## Full single-leaf comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `10A00004` | `10A00004` | `beqz a1,+4` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `90820007` | `90820007` | `lbu v0,7(a0)` |
| 3 | `0801DED5` | `0801DED5` | `j 0x80077B54` (local relocation normalized) |
| 4 | `34420001` | `34420001` | `ori v0,v0,1` |
| 5 | `90820007` | `90820007` | `lbu v0,7(a0)` |
| 6 | `00000000` | `00000000` | `nop` |
| 7 | `304200FE` | `304200FE` | `andi v0,v0,0xFE` |
| 8 | `03E00008` | `03E00008` | `jr ra` |
| 9 | `A0820007` | `A0820007` | `sb v0,7(a0)` |

Object disassembly before link:

```text
00000000 <func_80077B34>:
   0: 10a00004  beqz  a1,14
   4: 00000000  nop
   8: 90820007  lbu    v0,7(a0)
   c: 08000008  j      20
                    c: R_MIPS_26 .text
  10: 34420001  ori    v0,v0,0x1
  14: 90820007  lbu    v0,7(a0)
  18: 00000000  nop
  1c: 304200fe  andi   v0,v0,0xfe
  20: 03e00008  jr     ra
  24: a0820007  sb     v0,7(a0)
```

## Carve and packed-span proof

The prior asm span was `[0x6832C,0x68364)`, size `0x38`:

```text
prefix asm: 0x68334 - 0x6832C = 0x08
C leaf:     0x6835C - 0x68334 = 0x28
resume asm: 0x68364 - 0x6835C = 0x08
closure:    0x08 + 0x28 + 0x08 = 0x38
```

These sizes derive from span boundaries, not aligned object sizes. Full
packed span, file `0x68334..0x6835B`:

```text
retail:    0400a0100000000007008290d5de0108010042340700829000000000fe0042300800e003070082a0
candidate: 0400a0100000000007008290d5de0108010042340700829000000000fe0042300800e003070082a0
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh` exits
zero and reports 311 matching-C leaves.
