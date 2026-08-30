# `func_800C6ED8` — exact unsigned-halfword state setter

Outcome: **MATCHED** on the first natural C phrasing under era `-O2 -G0`.
Integrated as matching-C leaf 342 and as the first Tier-2 probe rung.

## Function hood and retail span

- File span: `[0xB76D8,0xB76E8)` = `0x10` bytes = four words.
- VRAM span: `[0x800C6ED8,0x800C6EE8)`.
- The body ends in canonical `jr ra; nop` at `0x800C6EE0/0x800C6EE4`.
- The preceding two words at file `0xB76D0/0xB76D4` are the real
  `jr ra; nop` ending `func_800C6EC0`.
- The following word at file `0xB76E8` is the real `lui at,0x800F` entry of
  `func_800C6EE8`.
- A raw executable scan for little-endian `B6 1B 03 0C` found exactly six
  direct `jal 0x800C6ED8` references:

| caller file PC | caller VA | word |
|---:|---:|---:|
| `0xC70BC` | `0x800D68BC` | `0x0C031BB6` |
| `0xCA5C8` | `0x800D9DC8` | `0x0C031BB6` |
| `0xCAD38` | `0x800DA538` | `0x0C031BB6` |
| `0xCCB6C` | `0x800DC36C` | `0x0C031BB6` |
| `0xCCD44` | `0x800DC544` | `0x0C031BB6` |
| `0xCEC20` | `0x800DE420` | `0x0C031BB6` |

Every caller supplies the value `1` in the call delay slot.
`FUNCTION_HOOD=PROVEN`: this is a callable function, not padding, data, or a
mis-split tail.

## Screens

| Screen | Result |
|---|---|
| Frame decomposition | no frame, locals, or saved registers |
| Callee buckets | none; no `jal` in the body |
| Stage-0 written global | `D_800F33E4` has this sole `sh` writer and one `lhu` reader at `0x800C720C`; unsigned halfword typing is closed |
| Coloring pressure | none; the only value is incoming `$a0` |
| `$v0` liveness | none; function is `void` and never writes `$v0` |
| Address retention | none; direct symbolic `sh` uses assembler macro temporary `$at` only |
| `-O` signal | optimized frame-free absolute store followed by canonical return; established era `-O2` setter shape |
| Loop/back-edge owner | none |
| Relocations | one normalized `R_MIPS_HI16/LO16` pair for `D_800F33E4` |

Flags: era `-O2 -G0`. No GP mode, three-word expansion, store-delay-slot
fill, division guard, dispatch fold, pins, or inline assembly is used. The
existing sb/sh screen predicted this exact pre-return store plus nop delay
slot; the maspsx store-fill gate deliberately does not alter halfword stores.

## Minimal C

```c
extern unsigned short D_800F33E4;

void func_800C6ED8(unsigned int value) {
    D_800F33E4 = (unsigned short)value;
}
```

## Single-leaf object: all four words

```text
00000000 <func_800C6ED8>:
   0: 3c010000  lui at,0x0       R_MIPS_HI16 D_800F33E4
   4: a4240000  sh  a0,0(at)     R_MIPS_LO16 D_800F33E4
   8: 03e00008  jr  ra
   c: 00000000  nop
```

After ordinary link relocation normalization:

```text
ROM:  3c01800f a42433e4 03e00008 00000000
C:    3c01800f a42433e4 03e00008 00000000

RELOCS_NORMALIZED=HI16/LO16(D_800F33E4)
BYTE_EXACT=4/4
```

## Carve geometry

Prior active asm span: `[0xB3368,0xB85C4)` = `0x525C`.

```text
asm prefix:  0xB76D8 - 0xB3368 = 0x4370
C leaf:      0xB76E8 - 0xB76D8 = 0x0010
resume asm:  0xB85C4 - 0xB76E8 = 0x0EDC
closure:     0x4370 + 0x0010 + 0x0EDC = 0x525C
```

The sizes come only from retail boundary arithmetic. Splat regenerated
`B3368.s` and `B76E8.s`; the trim guard accepted the boundary-derived sizes.

## Packed span and full gates

Packed candidate and retail are equal over `[0xB76D0,0xB76F0)`, including
both neighboring function boundaries:

```text
B76D0: 03e00008 = 03e00008  preceding function return
B76D4: 00000000 = 00000000  preceding real delay slot
B76D8: 3c01800f = 3c01800f  leaf word 1
B76DC: a42433e4 = a42433e4  leaf word 2
B76E0: 03e00008 = 03e00008  leaf word 3
B76E4: 00000000 = 00000000  leaf word 4
B76E8: 3c01800f = 3c01800f  following function entry
B76EC: a4243420 = a4243420  following function word 2
PACKED_SPAN=EXACT
```

```text
sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

scripts/verify_us.sh (tail)
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 342 leaves

grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
342
```

No pins, inline assembly, file-scope assembly, fabricated padding, or
mismatch-hiding mechanism is present.
