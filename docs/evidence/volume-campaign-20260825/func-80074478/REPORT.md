# `func_80074478` — exact indexed callback-table setter

Leaf 321, matched on the second bounded phrasing.

## Function hood and boundaries

Retail span `[0x64C78,0x64CA4)`, VRAM `0x80074478`, is eleven words and
ends in canonical `jr ra; nop`. It has no direct `jal`, but its exact start is
constructed as a function pointer:

```text
0x800743F4: lui    v0,%hi(func_80074478)
0x800743F8: addiu  v0,v0,%lo(func_80074478)
```

`func_800743B4` returns that pointer. Its exact caller at `0x80073EBC` stores
the returned value into a callback slot at `0x80073ECC`. This is one semantic
exact-start reference represented by the relocation pair, not two callers.

The preceding real `func_8007440C` ends at `0x80074470/0x80074474` with
`jr ra` and stack restoration. The following already-matched
`func_800744A4` starts at `0x800744A4` with `beqz a1`; both boundary words
are executable instructions and there is no padding in the leaf.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REFERENCE`.

## Screens and state provenance

| screen | result |
|---|---|
| callee buckets | no `jal`; conditional table setter |
| written-state Stage 0 | conditionally writes one of eight words in `D_8009568C`; `func_8007440C` reads the same eight entries and invokes each nonzero word with `jalr`, passing its index in `$a0` |
| coloring pressure | retail converts `$a0` from index to retained element address; `$a1` stays the proposed value; `$v0` changes from table base to old entry |
| `$v0` liveness | table base through address formation, then old value through the equality branch |
| address retention | computed element address remains in `$a0` across load, comparison, and optional store |
| optimization signal | explicit `lui/addiu`, load-delay nop, empty branch delay, and era register allocation select GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none; the eight-entry traversal belongs to reader `func_8007440C` |

The indirect calls prove that the stored words are callback addresses. The C
keeps an integer representation because the exact callback prototype is not
yet proven; pointers and integers are both 32-bit in this target ABI.

## C and bounded phrasings

Final C:

```c
extern int D_8009568C[];

void func_80074478(int index, int value) {
    if (value != D_8009568C[index]) {
        D_8009568C[index] = value;
    }
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; no pins, inline assembly,
or special maspsx switch.

Attempt 1 spelled the condition as `D_8009568C[index] != value`. It produced
the complete retail instruction shape and 10/11 matching words, but encoded
word 6 as `10450002` (`beq v0,a1`) instead of retail `10A20002`
(`beq a1,v0`). Reversing the commutative C operands in attempt 2 selected the
retail branch operand order and changed no other word.

## Full eleven-word comparison

| word | retail | candidate after relocation | instruction |
|---:|---:|---:|---|
| 0 | `3C028009` | `3C028009` | `lui v0,%hi(D_8009568C)` |
| 1 | `2442568C` | `2442568C` | `addiu v0,v0,%lo(D_8009568C)` |
| 2 | `00042080` | `00042080` | `sll a0,a0,2` |
| 3 | `00822021` | `00822021` | `addu a0,a0,v0` |
| 4 | `8C820000` | `8C820000` | `lw v0,0(a0)` |
| 5 | `00000000` | `00000000` | `nop` |
| 6 | `10A20002` | `10A20002` | `beq a1,v0,+2` |
| 7 | `00000000` | `00000000` | `nop` |
| 8 | `AC850000` | `AC850000` | `sw a1,0(a0)` |
| 9 | `03E00008` | `03E00008` | `jr ra` |
| 10 | `00000000` | `00000000` | `nop` |

Single-leaf object before relocation:

```text
00000000 <func_80074478>:
   0: 3c020000  lui    v0,0x0       R_MIPS_HI16 D_8009568C
   4: 24420000  addiu  v0,v0,0      R_MIPS_LO16 D_8009568C
   8: 00042080  sll    a0,a0,0x2
   c: 00822021  addu   a0,a0,v0
  10: 8c820000  lw     v0,0(a0)
  14: 00000000  nop
  18: 10a20002  beq    a1,v0,0x24
  1c: 00000000  nop
  20: ac850000  sw     a1,0(a0)
  24: 03e00008  jr     ra
  28: 00000000  nop
```

The object alignment word at `0x2C` is outside the boundary-derived body;
trim validation removes that zero word.

## Carve and packed-span proof

The prior asm span was `[0x64B54,0x64CA4)`, size `0x150`:

```text
prefix asm: 0x64C78 - 0x64B54 = 0x124
C leaf:     0x64CA4 - 0x64C78 = 0x02C
resume asm: 0x64CA4 - 0x64CA4 = 0x000
closure:    0x124 + 0x02C + 0x000 = 0x150
```

The zero-length resume is intentional: the leaf closes directly against the
existing C boundary for `func_800744A4`. Sizes come from file subtraction,
not aligned object size.

Packed candidate and retail match across both real-function boundaries:

```text
80074468: 8FB10014  lw     s1,20(sp)
8007446C: 8FB00010  lw     s0,16(sp)
80074470: 03E00008  jr     ra
80074474: 27BD0020  addiu  sp,sp,32
80074478: 3C028009  lui    v0,0x8009
8007447C: 2442568C  addiu  v0,v0,0x568C
80074480: 00042080  sll    a0,a0,2
80074484: 00822021  addu   a0,a0,v0
80074488: 8C820000  lw     v0,0(a0)
8007448C: 00000000  nop
80074490: 10A20002  beq    a1,v0,0x8007449C
80074494: 00000000  nop
80074498: AC850000  sw     a1,0(a0)
8007449C: 03E00008  jr     ra
800744A0: 00000000  nop
800744A4: 10A00006  beqz   a1,0x800744C0
800744A8: 24A2FFFF  addiu  v0,a1,-1
800744AC: 2403FFFF  addiu  v1,zero,-1
800744B0: AC800000  sw     zero,0(a0)
```

```text
retail:    0980023c8c56422480200400212082000000828c000000000200a21000000000000085ac0800e00300000000
candidate: 0980023c8c56422480200400212082000000828c000000000200a21000000000000085ac0800e00300000000
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
321
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 321 leaves
```

Result: `MATCHED=11/11`, second phrasing; consecutive-park count resets to
zero.
