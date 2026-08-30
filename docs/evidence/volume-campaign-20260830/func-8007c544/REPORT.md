# `func_8007C544` — exact three-word state setter

Outcome: **MATCHED** on natural C phrasing 1 under era `-O2 -G0` with the
established `MASPSX_FILL_STORE_DELAY_SLOT=1` gate. Integrated as matching-C
leaf 345 and Tier-2 probe rung 4.

## Function hood and retail span

- File `[0x6CD44,0x6CD60)`, VRAM `[0x8007C544,0x8007C560)`: `0x1C`
  bytes, seven words.
- Canonical return: `jr ra` at `0x8007C558`; the final `sw` is its live delay
  slot at `0x8007C55C`.
- One exact direct caller exists: `jal func_8007C544` at `0x8007C324`, with
  `a0 = 1` in its delay slot.
- The preceding real `func_8007C484` returns at file `0x6CD34/0x6CD38`,
  followed by two explicit alignment nops at `0x6CD3C/0x6CD40`.
- One alignment nop at `0x6CD60` separates this leaf from the real prologue of
  `func_8007C564` at `0x6CD64`.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALL`; the alignment words are not folded into
the body. This is not padding, data, or a tail entry.

## Retail body

```text
6CD44 8007C544 0C80013C  lui  at,%hi(D_800C0DC0)
6CD48 8007C548 C00D24AC  sw   a0,%lo(D_800C0DC0)(at)
6CD4C 8007C54C 0B80013C  lui  at,%hi(D_800B6918)
6CD50 8007C550 186925AC  sw   a1,%lo(D_800B6918)(at)
6CD54 8007C554 0C80013C  lui  at,%hi(D_800C0DBC)
6CD58 8007C558 0800E003  jr   ra
6CD5C 8007C55C BC0D26AC  sw   a2,%lo(D_800C0DBC)(at)
```

## Screens

| Screen | Result |
|---|---|
| Frame/callees | no frame, saved registers, calls, or callee buckets |
| Stage-0 globals | all three are zero-initialized 32-bit state; `D_800C0DC0` is read at `0x8007C838` and has later zero/computed writers, `D_800B6918` is read at `0x8007C84C`, and `D_800C0DBC` is read at `0x8007C4B4/CA0C/CB08` |
| Coloring pressure | none; `$a0/$a1/$a2` flow directly to ordered symbolic stores |
| `$v0` liveness | none; void return and no `$v0` use |
| Address retention | none; each absolute store gets a transient assembler `$at` materialization |
| `-O` signal | frame-free ordered stores and a useful return delay slot are the established era `-O2` setter pattern |
| Loop/back-edge | none |
| Relocations | three normalized `HI16/LO16` symbol pairs |

The retail `jr ra; sw` ending selects the existing default-off store-delay
gate. Without that gate, the same C emits `sw; jr ra; nop` (eight content
words); with it, only the independent last store moves into the return delay
slot and all seven retail words match. No compiler/maspsx code changed.

## Minimal C

```c
extern unsigned int D_800C0DC0;
extern unsigned int D_800B6918;
extern unsigned int D_800C0DBC;

void func_8007C544(unsigned int first, unsigned int second,
                   unsigned int third) {
    D_800C0DC0 = first;
    D_800B6918 = second;
    D_800C0DBC = third;
}
```

## Single-leaf object and ROM comparison

```text
00000000 <func_8007C544>:
   0: 3c010000  lui at,0       R_MIPS_HI16 D_800C0DC0
   4: ac240000  sw  a0,0(at)   R_MIPS_LO16 D_800C0DC0
   8: 3c010000  lui at,0       R_MIPS_HI16 D_800B6918
   c: ac250000  sw  a1,0(at)   R_MIPS_LO16 D_800B6918
  10: 3c010000  lui at,0       R_MIPS_HI16 D_800C0DBC
  14: 03e00008  jr  ra
  18: ac260000  sw  a2,0(at)   R_MIPS_LO16 D_800C0DBC

ROM: 3c01800c ac240dc0 3c01800b ac256918 3c01800c 03e00008 ac260dbc
C:   3c01800c ac240dc0 3c01800b ac256918 3c01800c 03e00008 ac260dbc
RELOCS_NORMALIZED=three HI16/LO16 pairs
BYTE_EXACT=7/7
```

## Carve geometry

The former active span was `[0x6C93C,0x6E6A4)` = `0x1D68`:

```text
asm prefix: 0x6CD44 - 0x6C93C = 0x0408
C leaf:     0x6CD60 - 0x6CD44 = 0x001C
asm resume: 0x6E6A4 - 0x6CD60 = 0x1944
closure:    0x0408 + 0x001C + 0x1944 = 0x1D68
```

All sizes are derived from boundaries, never aligned object sizes.

## Packed span and gates

```text
6CD34: 03e00008 = 03e00008  preceding return
6CD38: 00000000 = 00000000  preceding delay slot
6CD3C: 00000000 = 00000000  alignment
6CD40: 00000000 = 00000000  alignment
6CD44: 3c01800c = 3c01800c  leaf 1
6CD48: ac240dc0 = ac240dc0  leaf 2
6CD4C: 3c01800b = 3c01800b  leaf 3
6CD50: ac256918 = ac256918  leaf 4
6CD54: 3c01800c = 3c01800c  leaf 5
6CD58: 03e00008 = 03e00008  leaf 6
6CD5C: ac260dbc = ac260dbc  leaf 7 / return delay
6CD60: 00000000 = 00000000  alignment
6CD64: 27bdffc0 = 27bdffc0  following function entry
6CD68: 3c02800c = 3c02800c  following function word 2
PACKED_SPAN=EXACT
```

```text
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
scripts/verify_us.sh: exit 0; Split verification OK; compare EXACT MATCH
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 345
```
