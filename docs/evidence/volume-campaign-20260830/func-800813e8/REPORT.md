# `func_800813E8` — exact registered callback wrapper

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`; matching-C
leaf 367 and Tier-2 continuation rung 26.

## Function hood

- File `[0x71BE8,0x71C08)`, VA `[0x800813E8,0x80081408)`: eight words,
  ending in canonical `jr ra; nop`.
- There is no direct `jal` caller. `func_80081314` constructs the exact start
  at `0x80081364–0x80081368` and passes it at `0x8008136C` to
  `func_800824C8`, the proven callback-pointer exchange for `D_800B8AB4`.
- Real predecessor `func_80081314` ends at file `0x71BE0/0x71BE4`.
- The leaf is followed by three explicit alignment nops at
  `0x71C08..0x71C10`; real `func_80081414` begins at `0x71C14` with a
  nontrivial prologue.

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REGISTRATION`. The generated
label is not being used as function proof.

## Retail, screens, and C

```text
71BE8 800813E8 E8FFBD27  addiu sp,sp,-0x18
71BEC 800813EC 1000BFAF  sw    ra,0x10(sp)
71BF0 800813F0 59F1010C  jal   func_8007C564
71BF4 800813F4 00000000  nop
71BF8 800813F8 1000BF8F  lw    ra,0x10(sp)
71BFC 800813FC 1800BD27  addiu sp,sp,0x18
71C00 80081400 0800E003  jr    ra
71C04 80081404 00000000  nop
```

| Screen | Result |
|---|---|
| Callee | one unresolved `func_8007C564` call |
| Stage-0 | no direct global access; effects belong to the callee |
| Coloring / `$v0` | only `$ra`; call result is dead |
| Address retention | none |
| `-O` signal | canonical era `-O2` one-call frame; empty call delay slot |
| Loop | none |

```c
void func_8007C564(void);

void func_800813E8(void) {
    func_8007C564();
}
```

Flags: era `-O2 -G0`; no maspsx gate.

## Single-leaf object

```text
00000000 <func_800813E8>:
   0: 27bdffe8  addiu sp,sp,-24
   4: afbf0010  sw    ra,16(sp)
   8: 0c000000  jal   0                 R_MIPS_26 func_8007C564
   c: 00000000  nop
  10: 8fbf0010  lw    ra,16(sp)
  14: 27bd0018  addiu sp,sp,24
  18: 03e00008  jr    ra
  1c: 00000000  nop

ROM: 27bdffe8 afbf0010 0c01f159 00000000 8fbf0010 27bd0018 03e00008 00000000
C:   27bdffe8 afbf0010 0c01f159 00000000 8fbf0010 27bd0018 03e00008 00000000
RELOCS_NORMALIZED=one R_MIPS_26 call
BYTE_EXACT=8/8
```

## Carve geometry

Former asm `[0x71B10,0x71EF4)` = `0x03E4`:

```text
prefix: 0x71BE8 - 0x71B10 = 0x00D8
leaf:   0x71C08 - 0x71BE8 = 0x0020
resume: 0x71EF4 - 0x71C08 = 0x02EC
close:  0x00D8 + 0x0020 + 0x02EC = 0x03E4
```

These are boundary-derived sizes, never aligned object sizes.

## Packed span and gates

The packed candidate and retail words are identical:

```text
71BD8: 8fb1001c = 8fb1001c  preceding restore s1
71BDC: 8fb00018 = 8fb00018  preceding restore s0
71BE0: 03e00008 = 03e00008  preceding return
71BE4: 27bd0028 = 27bd0028  preceding teardown delay
71BE8: 27bdffe8 = 27bdffe8  leaf 1
71BEC: afbf0010 = afbf0010  leaf 2
71BF0: 0c01f159 = 0c01f159  leaf 3
71BF4: 00000000 = 00000000  leaf 4
71BF8: 8fbf0010 = 8fbf0010  leaf 5
71BFC: 27bd0018 = 27bd0018  leaf 6
71C00: 03e00008 = 03e00008  leaf 7
71C04: 00000000 = 00000000  leaf 8
71C08: 00000000 = 00000000  alignment nop 1
71C0C: 00000000 = 00000000  alignment nop 2
71C10: 00000000 = 00000000  alignment nop 3
71C14: 27bdffb0 = 27bdffb0  following real prologue
71C18: afb60048 = afb60048  following word 2
71C1C: 0080b021 = 0080b021  following word 3
71C20: afb3003c = afb3003c  following word 4
71C24: 00a09821 = 00a09821  following word 5
PACKED_SPAN=EXACT
```

```text
retail SHA-1:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
matching-C count: 367
```
