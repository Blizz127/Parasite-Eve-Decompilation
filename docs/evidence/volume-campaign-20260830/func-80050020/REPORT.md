# `func_80050020` — exact indexed callback getter

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0` with
the existing `MASPSX_THREE_WORD_SYMBOL_STORE=1` assembler gate. Integrated as
matching-C leaf 344 and Tier-2 probe rung 3.

## Function hood and retail span

- File `[0x40820,0x40838)`, VRAM `[0x80050020,0x80050038)`: `0x18`
  bytes, six words.
- Canonical return: `jr ra; nop` at `0x80050030/0x80050034`.
- Preceding real boundary: `func_8004FFF8` ends with `jr ra; nop` at file
  `0x40818/0x4081C`.
- Following real boundary: `func_80050038` starts with its stack-frame
  prologue at file `0x40838`.
- There is no direct `jal`, but the exact start is constructed at
  `0x80048420/0x80048424` and stored into an object callback slot at `+0x8C`
  in the branch delay slot at `0x8004842C`:

```text
38C20 80048420 0580023C  lui    v0,%hi(func_80050020)
38C24 80048424 20004224  addiu  v0,v0,%lo(func_80050020)
38C28 80048428 0B006010  beqz   v1,.L80048458
38C2C 8004842C 8C0082AC  sw     v0,0x8C(a0)
```

`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK`; the two pool references are
the relocation pair for this one logical callback assignment. This is not
padding, data, or a mislabeled tail.

## Retail body

```text
40820 80050020 00042080  sll   a0,a0,2
40824 80050024 0A80013C  lui   at,%hi(D_800A1888)
40828 80050028 21082400  addu  at,at,a0
4082C 8005002C 8818228C  lw    v0,%lo(D_800A1888)(at)
40830 80050030 0800E003  jr    ra
40834 80050034 00000000  nop
```

## Screens

| Screen | Result |
|---|---|
| Frame/callees | no frame, saved registers, calls, or callee buckets |
| Stage-0 global | `D_800A1888` begins a contiguous word-state run (`D_800A1888`, `...188C`, `...1890`, `...1894`); direct writers exist at `0x80044018` and `0x8004EE04`, while indexed readers/updates at `0x80047A98` and `0x80047AB0` independently prove array use |
| Coloring pressure | incoming `$a0` is scaled in place; the assembler's indexed-symbol expansion owns `$at`; loaded result naturally occupies `$v0` |
| `$v0` liveness | `$v0` is only the return value and remains live through `jr ra`; no competing temporary |
| Address retention | one indexed symbolic load; retail retains the expanded address in `$at` only through the load |
| `-O` signal | frame-free scale/address/load/return form and in-place argument scale are the established era `-O2` signal |
| Loop/back-edge | none |
| Relocations | one normalized `HI16/LO16` pair for `D_800A1888` |

Flags are era `-O2 -G0`. The existing default-off three-word gate is justified
by the retail form: `lui at; addu at,at,index; lw ...,LO16(at)`. This leaf
traverses the `$at` branch that is already required by the `func_800363F4`
class; it does not alter maspsx or the parked `func_8007FBF0` `$v0`-temporary
case. There are no pins, inline/file-scope assembly, or fabricated words.

## Minimal C

```c
extern unsigned int D_800A1888[];

unsigned int func_80050020(unsigned int index) {
    return D_800A1888[index];
}
```

## Single-leaf object and ROM comparison

```text
00000000 <func_80050020>:
   0: 00042080  sll   a0,a0,0x2
   4: 3c010000  lui   at,0x0       R_MIPS_HI16 D_800A1888
   8: 00240821  addu  at,at,a0
   c: 8c220000  lw    v0,0(at)     R_MIPS_LO16 D_800A1888
  10: 03e00008  jr    ra
  14: 00000000  nop

ROM: 00042080 3c01800a 00240821 8c221888 03e00008 00000000
C:   00042080 3c01800a 00240821 8c221888 03e00008 00000000
RELOCS_NORMALIZED=one HI16/LO16 pair
BYTE_EXACT=6/6
```

## Carve geometry

The former active span was `[0x40038,0x41518)` = `0x14E0`:

```text
asm prefix: 0x40820 - 0x40038 = 0x07E8
C leaf:     0x40838 - 0x40820 = 0x0018
asm resume: 0x41518 - 0x40838 = 0x0CE0
closure:    0x07E8 + 0x0018 + 0x0CE0 = 0x14E0
```

All sizes come from boundaries, never aligned object sizes.

## Packed span and gates

```text
40818: 03e00008 = 03e00008  preceding return
4081C: 00000000 = 00000000  preceding real delay slot
40820: 00042080 = 00042080  leaf 1
40824: 3c01800a = 3c01800a  leaf 2
40828: 00240821 = 00240821  leaf 3
4082C: 8c221888 = 8c221888  leaf 4
40830: 03e00008 = 03e00008  leaf 5
40834: 00000000 = 00000000  leaf 6
40838: 27bdffe8 = 27bdffe8  following function entry
4083C: afbf0010 = afbf0010  following function word 2
PACKED_SPAN=EXACT
```

```text
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 344
```
