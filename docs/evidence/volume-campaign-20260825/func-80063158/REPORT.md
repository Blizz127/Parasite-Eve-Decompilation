# `func_80063158` — exact null-safe local/global position accumulator

Leaf 332, matched on the first bounded phrasing after gp relocation
normalization.

## Function hood and boundary ownership

Retail span `[0x53958,0x53998)`, VRAM `[0x80063158,0x80063198)`, is
`0x40` bytes / sixteen words. It ends with canonical `jr ra; nop` at
`0x80063190/0x80063194`.

The executable contains 30 direct `jal 0x80063158` references:

```text
0x8004483C  0x800448A8  0x800460DC  0x80046410  0x80046534
0x800472A4  0x8004746C  0x80047488  0x800489B4  0x80048A8C
0x80048BD4  0x80048C20  0x80048E00  0x80048F90  0x80048FB8
0x80048FE0  0x8004927C  0x800492A4  0x800492CC  0x800498C8
0x80049B54  0x80049B78  0x8004A68C  0x8004BE04  0x8004E898
0x8004F038  0x8004F060  0x8004F088  0x8004F510  0x8004F560
```

The preceding real `func_80062FEC` ends with `jr ra; nop` at
`0x80063150/0x80063154`. The target begins immediately. It owns the return
delay slot through `0x80063194`; matched real `func_80063198` begins
immediately at `0x80063198` with a null guard. There is no boundary padding.

`FUNCTION_HOOD=PROVEN_BY_30_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Screens and written-state provenance

When the argument is non-null, the body adds `(x,y)` to the two words at
argument offsets `+0x18/+0x1C`, then adds the same pair to gp-relative globals
`D_8009D124/D_8009D128`.

The accepted `func_8005E8A4` evidence proves both globals as 32-bit paired
accumulators and records their reader census. Excluding this function itself,
31 other functions read one or both words:

```text
func_8005E8C4  func_8005EB64  func_8005EED4  func_8005F27C
func_8005F354  func_8005F5B8  func_8005F698  func_8005FA3C
func_8005FB74  func_8005FCAC  func_8005FDF0  func_8005FF28
func_8006006C  func_800602D0  func_80060528  func_8006055C
func_80060590  func_800605C4  func_800605F8  func_8006062C
func_80061A3C  func_80061B80  func_80061C34  func_80062090
func_800622BC  func_80062830  func_80062A7C  func_800634D4
func_800638D8  func_80064C80  func_80064EB4
```

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf updater |
| written-state Stage 0 | argument words `+0x18/+0x1C` and proven 32-bit gp pair `D_8009D124/D_8009D128`; 31 external readers above |
| coloring pressure | `$v0/$v1` carry paired old values and updated values; `$a1/$a2` remain the two deltas |
| `$v0` liveness | local `x` value, then global `x` value; no return value |
| address retention | fixed argument offsets plus direct gp-relative globals; no symbolic address register |
| optimization signal | interleaved local/global load-add-store schedule and gp-relative accesses select era GCC `-O2 -G8` |
| loop/back-edge owner | none |

## Final C and flags

```c
typedef struct {
    unsigned char pad[0x18];
    int x;
    int y;
} PositionNode;

extern int D_8009D124;
extern int D_8009D128;

void func_80063158(PositionNode *node, int x, int y) {
    if (node != 0) {
        node->x += x;
        node->y += y;
        D_8009D124 += x;
        D_8009D128 += y;
    }
}
```

Compiler: era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G8`. `_gp` is
`0x8009CD70`; therefore:

```text
D_8009D124 - _gp = 0x3B4
D_8009D128 - _gp = 0x3B8
```

No pins, inline assembly, or special maspsx switch is used. The first natural
phrasing is exact after applying the four ordinary `R_MIPS_GPREL16`
relocations.

## Full sixteen-word comparison

| word | retail | candidate normalized | instruction |
|---:|---:|---:|---|
| 0 | `1080000D` | `1080000D` | `beqz a0,+13` |
| 1 | `00000000` | `00000000` | `nop` |
| 2 | `8C820018` | `8C820018` | `lw v0,24(a0)` |
| 3 | `8C83001C` | `8C83001C` | `lw v1,28(a0)` |
| 4 | `00451021` | `00451021` | `addu v0,v0,a1` |
| 5 | `AC820018` | `AC820018` | `sw v0,24(a0)` |
| 6 | `8F8203B4` | `8F8203B4` | `lw v0,0x3B4(gp)` |
| 7 | `00661821` | `00661821` | `addu v1,v1,a2` |
| 8 | `AC83001C` | `AC83001C` | `sw v1,28(a0)` |
| 9 | `8F8303B8` | `8F8303B8` | `lw v1,0x3B8(gp)` |
| 10 | `00451021` | `00451021` | `addu v0,v0,a1` |
| 11 | `00661821` | `00661821` | `addu v1,v1,a2` |
| 12 | `AF8203B4` | `AF8203B4` | `sw v0,0x3B4(gp)` |
| 13 | `AF8303B8` | `AF8303B8` | `sw v1,0x3B8(gp)` |
| 14 | `03E00008` | `03E00008` | `jr ra` |
| 15 | `00000000` | `00000000` | `nop` |

Single-leaf object before relocation:

```text
00000000 <func_80063158>:
   0: 1080000d  beqz   a0,38
   4: 00000000  nop
   8: 8c820018  lw     v0,24(a0)
   c: 8c83001c  lw     v1,28(a0)
  10: 00451021  addu   v0,v0,a1
  14: ac820018  sw     v0,24(a0)
  18: 8f820000  lw     v0,0(gp)
      18: R_MIPS_GPREL16 D_8009D124
  1c: 00661821  addu   v1,v1,a2
  20: ac83001c  sw     v1,28(a0)
  24: 8f830000  lw     v1,0(gp)
      24: R_MIPS_GPREL16 D_8009D128
  28: 00451021  addu   v0,v0,a1
  2c: 00661821  addu   v1,v1,a2
  30: af820000  sw     v0,0(gp)
      30: R_MIPS_GPREL16 D_8009D124
  34: af830000  sw     v1,0(gp)
      34: R_MIPS_GPREL16 D_8009D128
  38: 03e00008  jr     ra
  3c: 00000000  nop
```

## Carve geometry and packed-span proof

The prior asm span was `[0x534E4,0x53998)`, size `0x4B4`:

```text
prefix asm:  0x53958 - 0x534E4 = 0x474
C leaf:      0x53998 - 0x53958 = 0x040
resume asm:  0x53998 - 0x53998 = 0x000
closure:     0x474 + 0x040 + 0x000 = 0x4B4
```

The leaf closes directly against the already matched next function. Retail
and packed candidate are identical through both real boundaries:

```text
retail:
00053940: 1800bf8f 1400b18f 1000b08f 2000bd27
00053950: 0800e003 00000000 0d008010 00000000
00053960: 1800828c 1c00838c 21104500 180082ac
00053970: b403828f 21186600 1c0083ac b803838f
00053980: 21104500 21186600 b40382af b80383af
00053990: 0800e003 00000000 02008010 00000000
000539a0: 480080ac 0800e003 00000000 02008010
000539b0: 01000224 480082ac 0800e003 00000000

candidate:
00053940: 1800bf8f 1400b18f 1000b08f 2000bd27
00053950: 0800e003 00000000 0d008010 00000000
00053960: 1800828c 1c00838c 21104500 180082ac
00053970: b403828f 21186600 1c0083ac b803838f
00053980: 21104500 21186600 b40382af b80383af
00053990: 0800e003 00000000 02008010 00000000
000539a0: 480080ac 0800e003 00000000 02008010
000539b0: 01000224 480082ac 0800e003 00000000
```

The packed target bytes equal the normalized single-leaf object:

```text
0d008010000000001800828c1c00838c21104500180082acb403828f21186600
1c0083acb803838f2110450021186600b40382afb80383af0800e00300000000
```

## Gates

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
volume file 0x53958 (63158): cand=0d008010000000001800828c1c00838c21104500180082acb403828f211866001c0083acb803838f2110450021186600b40382afb80383af0800e00300000000 orig=0d008010000000001800828c1c00838c21104500180082acb403828f211866001c0083acb803838f2110450021186600b40382afb80383af0800e00300000000
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 332 leaves
332
```

Result: `MATCHED=16/16` after relocation normalization, first phrasing;
consecutive parks remain zero.
