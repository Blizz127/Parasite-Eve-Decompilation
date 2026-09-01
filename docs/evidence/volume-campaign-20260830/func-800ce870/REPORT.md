# `func_800CE870` — matched Tier-1 position selector

Status: MATCHED, leaf 340. Exact on the second and final allowed natural-C
phrasing. No pins, inline assembly, file-scope assembly, forged padding, or
toolchain patch was used.

## Function hood and boundaries

The exact span is file `[0xBF070,0xBF0F0)`, VRAM
`[0x800CE870,0x800CE8F0)`: `0x80` bytes / 32 words. It ends with canonical
`jr ra; nop` at file `0xBF0E8/0xBF0EC`.

Twenty-seven distinct instructions call the exact start. Every call encodes
the same direct target (`0C033A1C`, file bytes `1C3A030C`):

```text
800D4CF4  800D4F14  800D76D8  800D77E4  800D7BDC  800D84B0
800D8600  800D8794  800D88E8  800D8C3C  800D8C7C  800D94C8
800D9780  800DA02C  800DAD5C  800DB134  800DBB14  800DC0D4
800DC7A4  800DCAD8  800DCF04  800DD208  800DD7C4  800DDDCC
800DE108  800DF8E4  800DFB00
```

Both boundary sides are real instructions. Preceding `func_800CE78C` ends
immediately with `jr ra; nop` at `0xBF068/0xBF06C`. Following handwritten
`func_800CE8F0` starts immediately at `0xBF0F0` with
`addiu sp,sp,-0x30`.

```text
FUNCTION_HOOD=PROVEN_BY_27_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine function, not padding, data, or a tail fragment.

## Retail semantics and screens

The second argument selects one of two three-component position sources. For
mode 0, the function follows the pointer at object offset `0x238` and copies
words `+0x14/+0x18/+0x1C`, truncating each into the caller's three-halfword
output. For mode 1, it converts three signed 16.16 object fields at
`+0x28/+0x2C/+0x30` to halfwords by taking their signed high halves. Other
mode values leave the output untouched.

The mode-1 instruction offsets are structural evidence: signed `lh` at
`+0x2A/+0x2E/+0x32` reads the high halfword of three aligned 32-bit fields.
They are not naturally modeled as three isolated halfword fields.

| screen | retail result | implication |
|---|---|---|
| callee buckets | no `jal`; no callees | leaf; no call-result or clobber bucket |
| frame / save pressure | no frame or saved registers | caller-saved allocation only |
| Stage-0 writers | no symbolic global; writes only three halfwords through `a2` | no global reader census; caller-owned output only |
| coloring pressure | object/mode/output stay in `a0/a1/a2`; `v0` is the sole value scratch | natural switch and field expressions are sufficient |
| `$v0` liveness | each branch leaves its final loaded component in `v0`, but all callers use the output buffer rather than a return value | void return; do not add an artificial result |
| address retention | mode 0 reloads `obj->source` before every component because intervening output stores may alias the object | preserve repeated source expressions rather than hoisting a pointer |
| optimization signal | two-case switch cascade, scheduled load-delay nops, and `signed int >> 16` folded to `lh` | era GCC 2.7.2 `-O2`; `-G0` is neutral here and matches this code region |
| loop/back-edge owner | none | acyclic selector |

## Two bounded phrasings

Phrasing 1 modeled object offsets `0x2A/0x2E/0x32` as standalone signed-short
fields. It recovered the complete control-flow and memory schedule, but cc1
correctly canonicalized three load/store truncations to `lhu`; retail uses
`lh`:

```text
off   phrasing 1  retail    residual
54    9482002A    8482002A  lhu -> lh
60    9482002E    8482002E  lhu -> lh
6C    94820032    84820032  lhu -> lh
```

The two local `j` words also differ before relocation, as expected; both have
`R_MIPS_26 .text` relocations and resolve exactly in the packed executable.

Phrasing 2 uses the evidence-supported signed 16.16 source fields. GCC folds
each arithmetic shift and truncating store into the retail signed high-half
load. This is the final C:

```c
typedef struct PositionSource {
    unsigned char pad_00[0x14];
    int field_14;
    int field_18;
    int field_1C;
} PositionSource;

typedef struct PositionObject {
    unsigned char pad_00[0x28];
    int field_28;
    int field_2C;
    int field_30;
    unsigned char pad_34[0x204];
    PositionSource *source;
} PositionObject;

void func_800CE870(PositionObject *obj, int mode, short *out)
{
    switch (mode) {
    case 0:
        out[0] = obj->source->field_14;
        out[1] = obj->source->field_18;
        out[2] = obj->source->field_1C;
        break;
    case 1:
        out[0] = obj->field_28 >> 16;
        out[1] = obj->field_2C >> 16;
        out[2] = obj->field_30 >> 16;
        break;
    }
}
```

Reusable lever:

```text
SIGNED-16.16-HIGH-HALF: when retail reads signed halfwords at +2 inside a
sequence of aligned four-byte fields, model the source as signed 32-bit fixed
point and spell the conversion as `field >> 16`. Era cc1 can fold the
arithmetic shift plus halfword destination into a single signed `lh` from the
high-half address. Modeling isolated short fields can instead canonicalize to
`lhu` because only the low 16 stored bits remain observable.
```

The exact invocation is era GCC 2.7.2 `-O2 -G0`, followed by ordinary
maspsx/ASPSX 2.21 and little-endian MIPS-I assembly. No opt-in maspsx gate is
enabled:

```sh
era_compile src/func_800CE870.c build/src/func_800CE870.c.o -O2 -G0
```

## Full 32-word object comparison

The standalone object has exactly two `.rel.text` entries:

```text
Offset     Type       symbol
00000010   R_MIPS_26  .text
0000004c   R_MIPS_26  .text
```

Normalizing those two local jumps from object-local `0800001E` to their
linked word `08033A3A` gives 32/32 exact words:

```text
off  object/raw  normalized  retail    instruction
00   10A00005    10A00005    10A00005  beqz a1,+5
04   24020001    24020001    24020001  addiu v0,zero,1
08   10A20012    10A20012    10A20012  beq a1,v0,+0x12
0C   00000000    00000000    00000000  nop
10   0800001E    08033A3A    08033A3A  j 800CE8E8
14   00000000    00000000    00000000  nop
18   8C820238    8C820238    8C820238  lw v0,0x238(a0)
1C   00000000    00000000    00000000  nop
20   8C420014    8C420014    8C420014  lw v0,0x14(v0)
24   00000000    00000000    00000000  nop
28   A4C20000    A4C20000    A4C20000  sh v0,0(a2)
2C   8C820238    8C820238    8C820238  lw v0,0x238(a0)
30   00000000    00000000    00000000  nop
34   8C420018    8C420018    8C420018  lw v0,0x18(v0)
38   00000000    00000000    00000000  nop
3C   A4C20002    A4C20002    A4C20002  sh v0,2(a2)
40   8C820238    8C820238    8C820238  lw v0,0x238(a0)
44   00000000    00000000    00000000  nop
48   8C42001C    8C42001C    8C42001C  lw v0,0x1C(v0)
4C   0800001E    08033A3A    08033A3A  j 800CE8E8
50   A4C20004    A4C20004    A4C20004  sh v0,4(a2)
54   8482002A    8482002A    8482002A  lh v0,0x2A(a0)
58   00000000    00000000    00000000  nop
5C   A4C20000    A4C20000    A4C20000  sh v0,0(a2)
60   8482002E    8482002E    8482002E  lh v0,0x2E(a0)
64   00000000    00000000    00000000  nop
68   A4C20002    A4C20002    A4C20002  sh v0,2(a2)
6C   84820032    84820032    84820032  lh v0,0x32(a0)
70   00000000    00000000    00000000  nop
74   A4C20004    A4C20004    A4C20004  sh v0,4(a2)
78   03E00008    03E00008    03E00008  jr ra
7C   00000000    00000000    00000000  nop
```

## Carve geometry

The former active assembly subsegment was `[0xBEC9C,0xC5050)`, size
`0x63B4`. Sizes come only from file-boundary arithmetic:

```text
prefix asm:  0xBF070 - 0xBEC9C = 0x03D4
C leaf:      0xBF0F0 - 0xBF070 = 0x0080
resume asm:  0xC5050 - 0xBF0F0 = 0x5F60
closure:     0x03D4 + 0x0080 + 0x5F60 = 0x63B4
```

The leaf is itself 16-byte aligned and its object `.text` is exactly `0x80`;
no aligned object size was used to derive any boundary.

## Full packed-span comparison

The final rebuilt and retail executables were independently disassembled as
raw little-endian MIPS over all 32 words at `[0xBF070,0xBF0F0)`. Both produce
this complete sequence:

```text
800CE870 10A00005  beqz a1,800CE888
800CE874 24020001  li v0,1
800CE878 10A20012  beq a1,v0,800CE8C4
800CE87C 00000000  nop
800CE880 08033A3A  j 800CE8E8
800CE884 00000000  nop
800CE888 8C820238  lw v0,0x238(a0)
800CE88C 00000000  nop
800CE890 8C420014  lw v0,0x14(v0)
800CE894 00000000  nop
800CE898 A4C20000  sh v0,0(a2)
800CE89C 8C820238  lw v0,0x238(a0)
800CE8A0 00000000  nop
800CE8A4 8C420018  lw v0,0x18(v0)
800CE8A8 00000000  nop
800CE8AC A4C20002  sh v0,2(a2)
800CE8B0 8C820238  lw v0,0x238(a0)
800CE8B4 00000000  nop
800CE8B8 8C42001C  lw v0,0x1C(v0)
800CE8BC 08033A3A  j 800CE8E8
800CE8C0 A4C20004  sh v0,4(a2)
800CE8C4 8482002A  lh v0,0x2A(a0)
800CE8C8 00000000  nop
800CE8CC A4C20000  sh v0,0(a2)
800CE8D0 8482002E  lh v0,0x2E(a0)
800CE8D4 00000000  nop
800CE8D8 A4C20002  sh v0,2(a2)
800CE8DC 84820032  lh v0,0x32(a0)
800CE8E0 00000000  nop
800CE8E4 A4C20004  sh v0,4(a2)
800CE8E8 03E00008  jr ra
800CE8EC 00000000  nop
```

Result: packed candidate and ROM are 32/32 exact, including both linked local
jump words.

## Full-build and verifier gates

Raw full-build tail:

```text
volume file 0xBF070 (CE870): cand=0500a010010002241200a210000000003a3a0308000000003802828c000000001400428c000000000000c2a43802828c000000001800428c000000000200c2a43802828c000000001c00428c3a3a03080400c2a42a008284000000000000c2a42e008284000000000200c2a432008284000000000400c2a40800e00300000000 orig=0500a010010002241200a210000000003a3a0308000000003802828c000000001400428c000000000000c2a43802828c000000001800428c000000000200c2a43802828c000000001c00428c3a3a03080400c2a42a008284000000000000c2a42e008284000000000200c2a432008284000000000400c2a40800e00300000000
RESULT: EXACT MATCH

=== Summary ===
Assemble: OK
Compile:  OK
Link:     OK
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH
```

Raw SHA gate:

```text
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/extracted/disc1/SLUS_006.62
```

Raw verifier tail and authoritative YAML count:

```text
=== Summary ===
Split verification (Phase 4E): OK.
Rebuild status (scripts/build_us.sh):
  scripts/build_us.sh: present
  candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 340 leaves (+ script VM step; era -O2 -G8 + EXPAND_DIV + THREE_WORD + DISPATCH_FOLD)
  source: src/func_80012850.c

$ grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
340
```

```text
MATCHING_C_COUNT=340
ACCEPTED_RESIDUAL_COUNT=20
```
