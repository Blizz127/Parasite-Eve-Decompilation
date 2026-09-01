# `func_800C2AF0` — exact base-publishing indexed setter

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0`, with no
maspsx behavior gate. Integrated as matching-C leaf 348 and Tier-2 probe rung
7.

## Function hood and retail span

- File `[0xB32F0,0xB3310)`, VRAM `[0x800C2AF0,0x800C2B10)`: `0x20`
  bytes, eight words.
- Canonical return: `jr ra` at `0x800C2B08`, with the indexed store as its
  live delay slot at `0x800C2B0C`.
- A raw executable scan finds eight unique exact direct callers:
  `0x800C7D14`, `0x800C8E58`, `0x800C9B50`, `0x800CA6E8`,
  `0x800CBEF4`, `0x800CCED0`, `0x800CD8B0`, and `0x800CE12C`.
- The preceding real `func_800C2758` ends with `jr ra; nop` at file
  `0xB32E8/0xB32EC`.
- The following real `func_800C2B10` begins immediately at file `0xB3310`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLS`. This is not padding, data, or a tail
entry.

## Retail body

```text
B32F0 800C2AF0 0C008424  addiu a0,a0,0xC
B32F4 800C2AF4 80300600  sll   a2,a2,2
B32F8 800C2AF8 2130C400  addu  a2,a2,a0
B32FC 800C2AFC 21100000  addu  v0,zero,zero
B3300 800C2B00 0E80013C  lui   at,%hi(D_800E2248)
B3304 800C2B04 482224AC  sw    a0,%lo(D_800E2248)(at)
B3308 800C2B08 0800E003  jr    ra
B330C 800C2B0C 4800C7AC  sw    a3,0x48(a2)
```

## Screens

| Screen | Result |
|---|---|
| Callee buckets | no calls or frame |
| Stage-0 global | `D_800E2248` is a mutable base pointer with four earlier direct writers in this active asm region and numerous field readers; adjacent accessors read offsets `+0x08`, `+0x48`, and `+0x70`, independently confirming base-pointer use |
| Coloring pressure | low; scaled index remains in `$a2`, advanced base remains in `$a0`, and value remains in `$a3` |
| `$v0` liveness | explicit zero result is established before either store and remains live through return |
| Address retention | the advanced base is both published globally and retained in `$a0` for indexed address formation |
| `-O` signal | frame-free strength reduction, explicit zero result, and useful return delay slot are the era `-O2` shape |
| Loop/back-edge | none |
| Relocations | one normalized `HI16/LO16` pair for `D_800E2248` |

The second formal is unused. The eight callers supply the remaining arguments
through the common callback wrapper shape; no stronger semantic names are
asserted.

## Minimal C and flags

```c
extern unsigned int *D_800E2248;

int func_800C2AF0(unsigned int *base, int unused, int index,
                  unsigned int value) {
    base += 3;
    D_800E2248 = base;
    base[index + 18] = value;
    return 0;
}
```

Compile: era cc1, `-O2 -G0`; no three-word, store-delay, div, or dispatch
gate. Cc1 itself schedules the independent indexed store into the return
delay slot.

## Single-leaf object and ROM comparison

```text
00000000 <func_800C2AF0>:
   0: 2484000c  addiu a0,a0,12
   4: 00063080  sll   a2,a2,2
   8: 00c43021  addu  a2,a2,a0
   c: 00001021  move  v0,zero
  10: 3c010000  lui   at,0             R_MIPS_HI16 D_800E2248
  14: ac240000  sw    a0,0(at)         R_MIPS_LO16 D_800E2248
  18: 03e00008  jr    ra
  1c: acc70048  sw    a3,72(a2)

ROM: 2484000c 00063080 00c43021 00001021 3c01800e ac242248 03e00008 acc70048
C:   2484000c 00063080 00c43021 00001021 3c01800e ac242248 03e00008 acc70048
RELOCS_NORMALIZED=one HI16/LO16 pair
BYTE_EXACT=8/8
```

## Carve geometry

The former active span was `[0xB2AF8,0xB3340)` = `0x0848`:

```text
asm prefix: 0xB32F0 - 0xB2AF8 = 0x07F8
C leaf:     0xB3310 - 0xB32F0 = 0x0020
asm resume: 0xB3340 - 0xB3310 = 0x0030
closure:    0x07F8 + 0x0020 + 0x0030 = 0x0848
```

All sizes come from split boundaries, never aligned object sizes.

## Packed span and gates

```text
B32E0: 8fb00020 = 8fb00020  preceding epilogue
B32E4: 27bd0048 = 27bd0048  preceding teardown
B32E8: 03e00008 = 03e00008  preceding return
B32EC: 00000000 = 00000000  preceding delay slot
B32F0: 2484000c = 2484000c  leaf 1
B32F4: 00063080 = 00063080  leaf 2
B32F8: 00c43021 = 00c43021  leaf 3
B32FC: 00001021 = 00001021  leaf 4
B3300: 3c01800e = 3c01800e  leaf 5
B3304: ac242248 = ac242248  leaf 6
B3308: 03e00008 = 03e00008  leaf 7
B330C: acc70048 = acc70048  leaf 8 / return delay
B3310: 00042080 = 00042080  following function entry
B3314: 3c02800e = 3c02800e  following function word 2
B3318: 8c422248 = 8c422248  following function word 3
B331C: 24840008 = 24840008  following function word 4
PACKED_SPAN=EXACT
```

```text
Compare:  EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 348
```
