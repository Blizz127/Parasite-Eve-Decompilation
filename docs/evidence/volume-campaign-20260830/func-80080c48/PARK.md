# func_80080C48 — bounded BCD load/scheduling residual

Status: `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING`.

Disposition: `ACCEPTED-RESIDUAL`. This is evidence disposition only, not a
byte-identity claim. The leaf remains assembly and does not increment the
matching-C count.

## Function hood and boundaries

The exact span is file `[0x71448,0x714C8)`, VRAM
`[0x80080C48,0x80080CC8)`: `0x80` bytes / 32 words. It ends with `jr ra` at
`0x80080CC0` and the live final `-150` arithmetic in the delay slot at
`0x80080CC4`.

Six distinct raw direct calls target the exact start:

```text
caller PC   file off  target
8007F14C    06F94C    80080C48
800699E0    05A1E0    80080C48
80069AB8    05A2B8    80080C48
80081EAC    0726AC    80080C48
80081EFC    0726FC    80080C48
80082224    072A24    80080C48
```

Each call is the direct `jal` word `0C020312` (little-endian file bytes
`12 03 02 0C`). The preceding real `func_80080B44` ends immediately at file
`0x71440/0x71444` with `jr ra` and a live byte store in its delay slot. The
following matched `func_80080CC8` begins immediately at file `0x714C8` with
a real instruction. Both boundary sides are executable.

```text
FUNCTION_HOOD=PROVEN_BY_6_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

## Semantics and screens

The function decodes the first three bytes of a CD `CdlLOC`-shaped record as
packed BCD minute, second, and frame fields, then converts them to a logical
block address:

```text
bcd(x) = (x >> 4) * 10 + (x & 0x0f)
result = ((bcd(position[0]) * 60 + bcd(position[1])) * 75)
       + bcd(position[2]) - 150
```

The inverse neighbor `func_80080B44`, the six libCD call contexts, and the
existing independent native implementation identify the semantics as
`CdPosToInt`. Read-only tests in `pc_port/tests/test_native.c` establish at
least these vectors:

```text
00:02:00 -> 0
01:00:00 -> 4350
09:59:74 -> 44849
```

No pc-port file was changed for this matching attempt.

| screen | retail result | consequence |
|---|---|---|
| callee buckets | zero `jal` in body | no callee result/clobber bucket |
| frame | no stack adjustment or saves | pure leaf; no prologue lever |
| Stage-0 globals | no symbolic read or write | no global census required |
| coloring pressure | input pointer, two prefetched bytes, BCD digit scratch, and one arithmetic accumulator | byte-load placement strongly controls register homes |
| `$v0` liveness | repeated strength-reduction scratch, then final accumulator and return | retail deliberately reuses `$v0` through each stage |
| address retention | `$a0` remains the input pointer until byte 2 is loaded at offset `0x54`, then becomes digit scratch | hoisting byte 2 kills the pointer early and changes later homes |
| `-O` signal | multiply-by-10/60/75 all strength-reduced; live return-slot subtract | era GCC 2.7.2 `-O2 -G0` is the justified probe |
| back-edge owner | none | straight-line leaf |

## Retail body — all 32 words

```text
off  word      instruction
00   90830000  lbu   v1,0(a0)
04   90860001  lbu   a2,1(a0)
08   00032902  srl   a1,v1,4
0C   00051080  sll   v0,a1,2
10   00451021  addu  v0,v0,a1
14   00021040  sll   v0,v0,1
18   3063000F  andi  v1,v1,0xf
1C   00431021  addu  v0,v0,v1
20   00022900  sll   a1,v0,4
24   00A22823  subu  a1,a1,v0
28   00052880  sll   a1,a1,2
2C   00061902  srl   v1,a2,4
30   00031080  sll   v0,v1,2
34   00431021  addu  v0,v0,v1
38   00021040  sll   v0,v0,1
3C   30C6000F  andi  a2,a2,0xf
40   00461021  addu  v0,v0,a2
44   00A22821  addu  a1,a1,v0
48   00051880  sll   v1,a1,2
4C   00651821  addu  v1,v1,a1
50   00031100  sll   v0,v1,4
54   90850002  lbu   a1,2(a0)
58   00431023  subu  v0,v0,v1
5C   00052102  srl   a0,a1,4
60   00041880  sll   v1,a0,2
64   00641821  addu  v1,v1,a0
68   00031840  sll   v1,v1,1
6C   30A5000F  andi  a1,a1,0xf
70   00651821  addu  v1,v1,a1
74   00431021  addu  v0,v0,v1
78   03E00008  jr    ra
7C   2442FF6A  addiu v0,v0,-150
```

## Two bounded C phrasings

Both attempts use era GCC 2.7.2 `-O2 -G0`, maspsx ASPSX 2.21 with
`--dont-expand-li`, and GNU MIPS-I `as`. There are no `.text` relocations.
No pin, inline asm, volatile ordering fiction, mismatch-hiding macro, or
forged nop was used.

Phrasing 1 exposes the three decoded fields as peer locals and combines them
in the return expression:

```c
int func_80080C48(const unsigned char *position)
{
    int minute;
    int second;
    int frame;

    minute = ((position[0] >> 4) * 10) + (position[0] & 0xF);
    second = ((position[1] >> 4) * 10) + (position[1] & 0xF);
    frame = ((position[2] >> 4) * 10) + (position[2] & 0xF);

    return (((minute * 60) + second) * 75) + frame - 150;
}
```

It emits exactly 32 words, but cc1 loads all three bytes at entry, computes
all three BCD values as peers, and only then combines them:

```text
90830000 90870001 90850002 00031102 00023080 00C23021 00063040 3063000F
00C33021 00071102 00021880 00621821 00031840 30E7000F 00671821 00051102
00022080 00822021 00042040 30A5000F 00852021 00061100 00461023 00021080
00431021 00021880 00621821 00031100 00431023 00441021 03E00008 2442FF6A
```

Positional equality is 3/32 (offsets `0x00`, `0x78`, and `0x7C`).

Phrasing 2 makes the arithmetic accumulator explicit and delays the frame
expression in source:

```c
int func_80080C48(const unsigned char *position)
{
    unsigned int minute;
    unsigned int second;
    unsigned int frame;
    int result;

    minute = ((position[0] >> 4) * 10) + (position[0] & 0xF);
    second = ((position[1] >> 4) * 10) + (position[1] & 0xF);
    result = (minute * 60) + second;
    result *= 75;

    frame = ((position[2] >> 4) * 10) + (position[2] & 0xF);
    result += frame;

    return result - 150;
}
```

It also emits exactly 32 words. It recovers retail's first two byte loads and
their `$v1/$a2` homes, but still hoists byte 2 into `$a0` at entry and keeps
the BCD calculations ahead of the accumulator stages:

```text
90830000 90860001 90840002 00031102 00022880 00A22821 00052840 3063000F
00A32821 00061102 00021880 00621821 00031840 30C6000F 00661821 00051100
00451023 00021080 00432821 00051080 00451021 00021900 00622823 00041902
00031080 00431021 00021040 3084000F 00441021 00A22821 03E00008 24A2FF6A
```

Positional equality is 3/32 (offsets `0x00`, `0x04`, and `0x78`). The first
residual is at offset `0x08`: retail starts byte-0 BCD arithmetic while the
candidate executes the hoisted `lbu a0,2(a0)`. Because that instruction makes
the pointer dead immediately, every subsequent register home and strength-
reduction schedule diverges, including the source register of the otherwise
correct `-150` return-slot operation.

This is the same recognized broad cc1 behavior as the independent-load
scheduling residual at `func_8006E454`: an independent later byte load is
hoisted across intervening arithmetic. Here the hoist additionally changes
the BCD accumulator's register homes and expression schedule. The two-source
phrasing budget is exhausted; imposing `volatile` would assert unsupported
I/O semantics solely to constrain code generation.

```text
RESIDUAL_MECHANISM=INDEPENDENT_LOAD_AND_BCD_ACCUMULATOR_SCHEDULING
FUTURE_UNBLOCKER=a compiler scheduler/allocation lever that keeps byte 2 at
  its retail use point and pipelines each decoded BCD field into the running
  minute/second/frame accumulator
```

The final candidate is preserved in labeled stash
`park func_80080C48 independent-load scheduling residual`; recover the
untracked source from that stash's third parent. Both attempt objects remain
under `build/attempts/func_80080C48/` locally. There is no YAML, build, or
verifier integration. The exact matching-C count remains 340.
