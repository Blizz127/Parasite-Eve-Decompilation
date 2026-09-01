# `func_8008788C` — exact adjacent SPU bitfield update

Leaf 324, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x7808C,0x780C0)`, VRAM `[0x8008788C,0x800878C0)`, is
`0x34` bytes / thirteen words. It ends in canonical `jr ra` with its
halfword store in the return delay slot. Exact direct callers occur at
`0x800879E8`, `0x8008C7AC`, and `0x8008C98C`.

The preceding real `func_80087864` ends at `0x80087884/0x80087888` with
`jr ra` and a halfword-store delay slot. The following matched
`func_800878C0` begins exactly at `0x800878C0` with `lui v0,0x1F80`.
Both boundaries are executable instructions; there is no padding ambiguity.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Screens

| screen | result |
|---|---|
| callee buckets | no `jal`; MMIO read/modify/write leaf |
| written-state Stage 0 | no RAM global; writes the per-voice SPU halfword at `0x1F801C0A + voice*0x10` |
| coloring pressure | `$a0` becomes the retained MMIO address; `$a2` and `$a1` become the upper fields; `$v0` carries base, old halfword, and final value |
| `$v0` liveness | fixed base → loaded halfword → masked/final value through return-slot store |
| address retention | computed MMIO pointer remains in `$a0` from `lhu` through `sh` |
| optimization signal | fixed-address `lui; ori`, grouped OR tree, in-place shifts, and return-delay store select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

The hardware address proves another per-voice SPU control-register update;
its layout is consistent with the second ADSR halfword. No exact Psy-Q
routine name is assigned without symbol evidence. The operation preserves
bits 0–5 and combines `(field << 6)` with `(mode >> 1) << 14`.

## C and flags

```c
void func_8008788C(int voice, unsigned int field, unsigned int mode) {
    unsigned short *reg =
        (unsigned short *)(0x1F801C0A + (voice << 4));

    *reg = (unsigned short)((*reg & 0x003F) |
                            (((mode >> 1) << 14) | (field << 6)));
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`; first phrasing, with no pins,
inline assembly, or special maspsx switch. The grouped fields select retail's
`$a2 |= $a1` DAG, while the nonvolatile lvalue produces the retail `jr; sh`
ending through ordinary compiler scheduling.

## Full thirteen-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `3C021F80` | `3C021F80` | `lui v0,0x1F80` |
| 1 | `34421C0A` | `34421C0A` | `ori v0,v0,0x1C0A` |
| 2 | `00042100` | `00042100` | `sll a0,a0,4` |
| 3 | `00822021` | `00822021` | `addu a0,a0,v0` |
| 4 | `00063042` | `00063042` | `srl a2,a2,1` |
| 5 | `00063380` | `00063380` | `sll a2,a2,14` |
| 6 | `00052980` | `00052980` | `sll a1,a1,6` |
| 7 | `94820000` | `94820000` | `lhu v0,0(a0)` |
| 8 | `00C53025` | `00C53025` | `or a2,a2,a1` |
| 9 | `3042003F` | `3042003F` | `andi v0,v0,0x003F` |
| 10 | `00461025` | `00461025` | `or v0,v0,a2` |
| 11 | `03E00008` | `03E00008` | `jr ra` |
| 12 | `A4820000` | `A4820000` | `sh v0,0(a0)` |

Single-leaf object; there are no relocations to normalize:

```text
00000000 <func_8008788C>:
   0: 3c021f80  lui    v0,0x1f80
   4: 34421c0a  ori    v0,v0,0x1c0a
   8: 00042100  sll    a0,a0,0x4
   c: 00822021  addu   a0,a0,v0
  10: 00063042  srl    a2,a2,0x1
  14: 00063380  sll    a2,a2,0xe
  18: 00052980  sll    a1,a1,0x6
  1c: 94820000  lhu    v0,0(a0)
  20: 00c53025  or     a2,a2,a1
  24: 3042003f  andi   v0,v0,0x3f
  28: 00461025  or     v0,v0,a2
  2c: 03e00008  jr     ra
  30: a4820000  sh     v0,0(a0)
```

Gas aligns `.text` to `0x40`; the trim guard proved all twelve bytes beyond
the boundary-derived `0x34` body are zero before removing them.

## Carve geometry

The prior asm span was `[0x7800C,0x780C0)`, size `0x0B4`:

```text
prefix asm:  0x7808C - 0x7800C = 0x080
C leaf:      0x780C0 - 0x7808C = 0x034
resume asm:  0x780C0 - 0x780C0 = 0x000
closure:     0x080 + 0x034 + 0x000 = 0x0B4
```

The zero-length resume closes directly against the existing C boundary for
`func_800878C0`. Sizes come from file subtraction, not object alignment.

## Packed-span proof

Retail and candidate disassemble identically across both real-function
boundaries:

```text
8008787C: 3042FFF0  andi   v0,v0,0xFFF0
80087880: 00451025  or     v0,v0,a1
80087884: 03E00008  jr     ra
80087888: A4820000  sh     v0,0(a0)
8008788C: 3C021F80  lui    v0,0x1F80
80087890: 34421C0A  ori    v0,v0,0x1C0A
80087894: 00042100  sll    a0,a0,4
80087898: 00822021  addu   a0,a0,v0
8008789C: 00063042  srl    a2,a2,1
800878A0: 00063380  sll    a2,a2,14
800878A4: 00052980  sll    a1,a1,6
800878A8: 94820000  lhu    v0,0(a0)
800878AC: 00C53025  or     a2,a2,a1
800878B0: 3042003F  andi   v0,v0,0x003F
800878B4: 00461025  or     v0,v0,a2
800878B8: 03E00008  jr     ra
800878BC: A4820000  sh     v0,0(a0)
800878C0: 3C021F80  lui    v0,0x1F80
800878C4: 34421C0A  ori    v0,v0,0x1C0A
800878C8: 00042100  sll    a0,a0,4
800878CC: 00822021  addu   a0,a0,v0
```

```text
retail:    00000000f0ff4230251045000800e003000082a4801f023c0a1c42340021040021208200423006008033060080290500000082942530c5003f004230251046000800e003000082a4801f023c0a1c42340021040021208200
candidate: 00000000f0ff4230251045000800e003000082a4801f023c0a1c42340021040021208200423006008033060080290500000082942530c5003f004230251046000800e003000082a4801f023c0a1c42340021040021208200
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
324
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 324 leaves
```

Result: `MATCHED=13/13`, first phrasing; consecutive parks remain zero.
