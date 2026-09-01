# `func_800749D8` — exact display-environment initializer

Leaf 331, matched on the first bounded phrasing.

## Function hood and boundary ownership

Retail span `[0x651D8,0x65214)`, VRAM `[0x800749D8,0x80074A14)`, is
`0x3C` bytes / fifteen words. It ends with canonical `jr ra` and the final
height store in its delay slot at `0x80074A0C/0x80074A10`.

The executable contains four direct `jal 0x800749D8` references:

```text
0x8003E7E8  0x8003E800  0x8005E664  0x8005E67C
```

The preceding real `func_80074924` ends immediately before the target with
`jr ra; addiu sp,sp,0x28` at `0x800749D0/0x800749D4`. The target owns its
live return delay slot through `0x80074A10`; matched real `func_80074A14`
then begins immediately with a symbolic `lui`. There is no padding inside
the target or between either real boundary.

`FUNCTION_HOOD=PROVEN_BY_4_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Semantic classification and screens

The writes prove a 20-byte display-environment layout: a four-halfword
display rectangle at `+0x00`, a zeroed four-halfword screen rectangle at
`+0x08`, and four zeroed control/padding bytes at `+0x10..+0x13`. The first
four call arguments supply display `x/y/w`; the o32 stack argument supplies
display `h`. The input pointer is returned.

This is body-level proof of a display-environment initializer. Its position
and exact Psy-Q `DISPENV` layout make `SetDefDispEnv` the probable SDK name,
but that name remains an inference rather than a string- or symbol-proven
attribution.

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | writes only the caller-supplied 20-byte environment; no global state |
| coloring pressure | `$v0` retains and returns the environment base; `$v1` retains the fifth stack argument until the return delay slot |
| `$v0` liveness | argument pointer copied at entry, used as every store base, and returned unchanged |
| address retention | argument-relative stores only; the retained `$v0` base is naturally induced by the returned pointer |
| optimization signal | fifth argument loaded first and its independent store sunk into the return delay slot select era GCC `-O2 -G0` |
| loop/back-edge owner | none |

## Final C and flags

```c
typedef struct {
    short x;
    short y;
    short w;
    short h;
    short screen_x;
    short screen_y;
    short screen_w;
    short screen_h;
    unsigned char isinter;
    unsigned char isrgb24;
    unsigned char pad0;
    unsigned char pad1;
} DisplayEnv;

DisplayEnv *func_800749D8(DisplayEnv *env, int x, int y, int w, int h) {
    env->x = x;
    env->y = y;
    env->w = w;
    env->h = h;
    env->screen_x = 0;
    env->screen_y = 0;
    env->screen_w = 0;
    env->screen_h = 0;
    env->isrgb24 = 0;
    env->isinter = 0;
    env->pad1 = 0;
    env->pad0 = 0;
    return env;
}
```

Compiler: era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`. No pins,
inline assembly, relocation normalization, or maspsx feature gate is used.
The first natural struct phrasing is byte-exact.

## Full fifteen-word comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `8FA30010` | `8FA30010` | `lw v1,16(sp)` |
| 1 | `00801021` | `00801021` | `move v0,a0` |
| 2 | `A4450000` | `A4450000` | `sh a1,0(v0)` |
| 3 | `A4460002` | `A4460002` | `sh a2,2(v0)` |
| 4 | `A4470004` | `A4470004` | `sh a3,4(v0)` |
| 5 | `A4400008` | `A4400008` | `sh zero,8(v0)` |
| 6 | `A440000A` | `A440000A` | `sh zero,10(v0)` |
| 7 | `A440000C` | `A440000C` | `sh zero,12(v0)` |
| 8 | `A440000E` | `A440000E` | `sh zero,14(v0)` |
| 9 | `A0400011` | `A0400011` | `sb zero,17(v0)` |
| 10 | `A0400010` | `A0400010` | `sb zero,16(v0)` |
| 11 | `A0400013` | `A0400013` | `sb zero,19(v0)` |
| 12 | `A0400012` | `A0400012` | `sb zero,18(v0)` |
| 13 | `03E00008` | `03E00008` | `jr ra` |
| 14 | `A4430006` | `A4430006` | `sh v1,6(v0)` |

Single-leaf object:

```text
00000000 <func_800749D8>:
   0: 8fa30010  lw     v1,16(sp)
   4: 00801021  move   v0,a0
   8: a4450000  sh     a1,0(v0)
   c: a4460002  sh     a2,2(v0)
  10: a4470004  sh     a3,4(v0)
  14: a4400008  sh     zero,8(v0)
  18: a440000a  sh     zero,10(v0)
  1c: a440000c  sh     zero,12(v0)
  20: a440000e  sh     zero,14(v0)
  24: a0400011  sb     zero,17(v0)
  28: a0400010  sb     zero,16(v0)
  2c: a0400013  sb     zero,19(v0)
  30: a0400012  sb     zero,18(v0)
  34: 03e00008  jr     ra
  38: a4430006  sh     v1,6(v0)
```

## Carve geometry and packed-span proof

The prior asm span was `[0x64F70,0x65214)`, size `0x2A4`:

```text
prefix asm:  0x651D8 - 0x64F70 = 0x268
C leaf:      0x65214 - 0x651D8 = 0x03C
resume asm:  0x65214 - 0x65214 = 0x000
closure:     0x268 + 0x03C + 0x000 = 0x2A4
```

The leaf closes directly against the already matched next function. Retail
and packed candidate are identical through both real boundaries:

```text
retail:
000651c8: 1400b18f 1000b08f 0800e003 2800bd27
000651d8: 1000a38f 21108000 000045a4 020046a4
000651e8: 040047a4 080040a4 0a0040a4 0c0040a4
000651f8: 0e0040a4 110040a0 100040a0 130040a0
00065208: 120040a0 0800e003 060043a4 0980023c
00065218: ec56428c 0980013c 0800e003 ec5624ac

candidate:
000651c8: 1400b18f 1000b08f 0800e003 2800bd27
000651d8: 1000a38f 21108000 000045a4 020046a4
000651e8: 040047a4 080040a4 0a0040a4 0c0040a4
000651f8: 0e0040a4 110040a0 100040a0 130040a0
00065208: 120040a0 0800e003 060043a4 0980023c
00065218: ec56428c 0980013c 0800e003 ec5624ac
```

The packed target bytes equal the single-leaf object:

```text
1000a38f21108000000045a4020046a4040047a4080040a40a0040a40c0040a4
0e0040a4110040a0100040a0130040a0120040a00800e003060043a4
```

## Build guard and gates

The first integrated build was intentionally stopped by the trim guard
because ignored `asm/disc1/64F70.s` was stale and still contained the carved
leaf:

```text
ERROR: build/asm/disc1/64F70.s.o .text: bytes beyond 0x268 are not all zero
(first nonzero evidence — refusing silent truncate)
```

After regenerating ignored split artifacts from the edited YAML, the same
geometry passed (`64F70.s.o 0x270→0x268`, zero pad only) and all gates were
exact:

```text
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
volume file 0x651D8 (749D8): cand=1000a38f21108000000045a4020046a4040047a4080040a40a0040a40c0040a40e0040a4110040a0100040a0130040a0120040a00800e003060043a4 orig=1000a38f21108000000045a4020046a4040047a4080040a40a0040a40c0040a40e0040a4110040a0100040a0130040a0120040a00800e003060043a4
Split verification (Phase 4E): OK.
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 331 leaves
331
```

Result: `MATCHED=15/15`, first phrasing; consecutive parks remain zero.
