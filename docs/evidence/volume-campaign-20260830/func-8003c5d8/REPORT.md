# func_8003C5D8 — matched Tier-1 leaf

Status: MATCHED, leaf 336. Two bounded phrasings. No pins, inline assembly,
volatile scheduling tricks, or forged nops.

## Function hood and boundaries

The exact span is file `[0x2CDD8,0x2CE38)`, VRAM
`[0x8003C5D8,0x8003C638)`: `0x60` bytes / 24 words. It ends in the
canonical return `jr ra` at file `0x2CE30`, with a real byte store in its
delay slot at `0x2CE34`.

There are 26 distinct direct `jal 0x8003C5D8` callers:

```text
80015060 800150B8 800190DC 8001911C 80024AF4 80024B18
80024BF0 80024EE8 80025150 80025278 80025328 800253F8
80025424 80028F7C 80028FEC 8002AB9C 8002ABBC 8002AC88
8002ACCC 8002B4CC 8002B798 8002B7BC 8002BA00 8002BAA0
8002F1B0 8003D764
```

Both boundaries are executable instructions. The preceding real function
ends with `jr ra; nop` at file `0x2CDD0/0x2CDD4`. The following function
begins immediately at file `0x2CE38` with `addiu sp,sp,-0x28`
(`0x27BDFFD8`). Therefore:

```text
FUNCTION_HOOD=PROVEN_BY_26_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine callable function, not alignment, data, or a mislabeled
tail entry.

## Retail semantics and screens

The routine accepts a state pointer and a signed 16-bit scale. A zero scale
is replaced by one. It computes signed `128 / scale`, stores the selected
scale byte at state offset `0x8D`, and stores the reciprocal byte at offsets
`0x8E`, `0x8F`, and `0x93`.

Independent semantic evidence already exists in
`docs/evidence/pe-btl6-3d050-remainder/REPORT.md` and the read-only native
oracle `pc_port/game/boot/func_8003C5D8_port.c`: zero selects one, division
uses the signed 16-bit value, and the four field effects agree.

| screen | retail result | implication |
|---|---|---|
| callees / buckets | no `jal`; no callees | leaf; no callee-side return or clobber buckets |
| Stage-0 writers | no symbolic global write; only `a0+0x8D/8E/8F/93` | no global reader census required; pointer-owned state |
| coloring pressure | input copied `a1 -> a2`; divisor in `v0`; quotient in `v1` | signed-short promotion must preserve the incoming-value copy |
| `$v0` liveness | divisor lives in `v0` through the signed-div guards; result is void | do not shape a C return value |
| address retention | none; every store is argument-relative | no symbolic-address lever or assembler gate |
| optimization signal | scheduled branch slots, no frame, immediate `128` | era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none | straight-line guarded division |

Retail's `break 7` divide-by-zero guard and `break 6` `INT_MIN / -1` guard
identify the established ASPSX signed-division expansion. This justifies the
per-leaf `MASPSX_EXPAND_DIV=1` gate; no other maspsx gate is enabled.

## Bounded phrasing attempts

Attempt 1 used an `int scale` formal and an explicit reciprocal local. It
compiled to the same aligned `0x60` object size but only 23 content words plus
alignment. GCC reused `$a1`, hoisted the sign extension into the guard delay
slot, and omitted retail's entry copy:

```text
00000000 00051400  sll   v0,a1,16
00000004 14400004  bnez  v0,0x18
00000008 00021403  sra   v0,v0,16
0000000C 24050001  li    a1,1
...
00000048 A085008D  sb    a1,0x8D(a0)
00000054 03E00008  jr    ra
00000058 A0830093  sb    v1,0x93(a0)
0000005C 00000000  nop
```

The first word already differs from retail's `move a2,a1`; all 24 positions
therefore miss. This is a formal-width/code-shape mismatch, not toolchain
skew.

Attempt 2 changes only the proven parameter type to signed `short`. The ABI
still passes it in `$a1`, while cc1 must preserve the original value in `$a2`
before implementing signed-16 promotion. That recovers all 24 retail words.

## Final C

```c
void func_8003C5D8(unsigned char *state, short scale)
{
    int reciprocal;

    if (scale == 0) {
        scale = 1;
    }

    reciprocal = 128 / scale;
    state[0x8D] = scale;
    state[0x8E] = reciprocal;
    state[0x8F] = reciprocal;
    state[0x93] = reciprocal;
}
```

The exact invocation is era GCC 2.7.2 `-O2 -G0`, maspsx ASPSX 2.21 with
`--dont-expand-li --expand-div`, then GNU `as` for little-endian MIPS I. In
the build script this is:

```sh
MASPSX_EXPAND_DIV=1 era_compile src/func_8003C5D8.c \
  build/src/func_8003C5D8.c.o -O2 -G0
```

## Full 24-word object comparison

There are no symbol relocations in this leaf; local branch offsets are fully
resolved. `mipsel-linux-gnu-objdump -dr` gives:

```text
off   candidate  retail    instruction
00    00A03021   00A03021  move  a2,a1
04    00052C00   00052C00  sll   a1,a1,16
08    14A00003   14A00003  bnez  a1,0x18
0C    00061400   00061400  sll   v0,a2,16
10    24060001   24060001  li    a2,1
14    00061400   00061400  sll   v0,a2,16
18    00021403   00021403  sra   v0,v0,16
1C    24030080   24030080  li    v1,128
20    0062001A   0062001A  div   zero,v1,v0
24    14400002   14400002  bnez  v0,0x30
28    00000000   00000000  nop
2C    0007000D   0007000D  break 7
30    2401FFFF   2401FFFF  li    at,-1
34    14410004   14410004  bne   v0,at,0x48
38    3C018000   3C018000  lui   at,0x8000
3C    14610002   14610002  bne   v1,at,0x48
40    00000000   00000000  nop
44    0006000D   0006000D  break 6
48    00001812   00001812  mflo  v1
4C    A086008D   A086008D  sb    a2,0x8D(a0)
50    A083008E   A083008E  sb    v1,0x8E(a0)
54    A083008F   A083008F  sb    v1,0x8F(a0)
58    03E00008   03E00008  jr    ra
5C    A0830093   A0830093  sb    v1,0x93(a0)
```

Result: `24/24`, zero mismatches.

## Carve geometry

The former assembly subsegment was `[0x29574,0x2E02C)`, size `0x4AB8`.
Sizes come from file-boundary arithmetic, never aligned object sizes:

```text
prefix asm: 0x2CDD8 - 0x29574 = 0x3864
C leaf:     0x2CE38 - 0x2CDD8 = 0x0060
resume asm: 0x2E02C - 0x2CE38 = 0x11F4
closure:    0x3864 + 0x0060 + 0x11F4 = 0x4AB8
```

The trim guard reports the C object's `.text` as `0x60 -> 0x60 (-0)` and
accepts both asm boundary sizes, so no nonzero body byte is truncated.

## Full packed-span comparison

This compares the rebuilt executable against retail from the preceding
function's return through the first word of the following function:

```text
file    VRAM       candidate retail   status
2CDD0   8003C5D0   03E00008  03E00008 MATCH
2CDD4   8003C5D4   00000000  00000000 MATCH
2CDD8   8003C5D8   00A03021  00A03021 MATCH
2CDDC   8003C5DC   00052C00  00052C00 MATCH
2CDE0   8003C5E0   14A00003  14A00003 MATCH
2CDE4   8003C5E4   00061400  00061400 MATCH
2CDE8   8003C5E8   24060001  24060001 MATCH
2CDEC   8003C5EC   00061400  00061400 MATCH
2CDF0   8003C5F0   00021403  00021403 MATCH
2CDF4   8003C5F4   24030080  24030080 MATCH
2CDF8   8003C5F8   0062001A  0062001A MATCH
2CDFC   8003C5FC   14400002  14400002 MATCH
2CE00   8003C600   00000000  00000000 MATCH
2CE04   8003C604   0007000D  0007000D MATCH
2CE08   8003C608   2401FFFF  2401FFFF MATCH
2CE0C   8003C60C   14410004  14410004 MATCH
2CE10   8003C610   3C018000  3C018000 MATCH
2CE14   8003C614   14610002  14610002 MATCH
2CE18   8003C618   00000000  00000000 MATCH
2CE1C   8003C61C   0006000D  0006000D MATCH
2CE20   8003C620   00001812  00001812 MATCH
2CE24   8003C624   A086008D  A086008D MATCH
2CE28   8003C628   A083008E  A083008E MATCH
2CE2C   8003C62C   A083008F  A083008F MATCH
2CE30   8003C630   03E00008  03E00008 MATCH
2CE34   8003C634   A0830093  A0830093 MATCH
2CE38   8003C638   27BDFFD8  27BDFFD8 MATCH
```

`packed_equal=True` for all `0x6C` displayed bytes, and `leaf_equal=True`
for the full `0x60` leaf. The packed bytes agree with the isolated object.

## Full gates

```text
$ sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

$ scripts/verify_us.sh
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 336 leaves

$ grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
336
```

The refreshed active pool is now 1,093 spans: Tier 1 14, Tier 2 214, Tier 3
99, and SKIP/suppressed 766. Consecutive parks remain zero.
