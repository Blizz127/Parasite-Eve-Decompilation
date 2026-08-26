# func_8006DB48 — matched Tier-1 leaf

Status: MATCHED, leaf 335. No pins or inline assembly.

## Function hood and boundaries

The span is file [0x5E348,0x5E39C), VRAM [0x8006DB48,0x8006DB9C), 0x54
bytes / 21 words. It ends with jr ra at 0x8006DB94 and its delay-slot nop
at 0x8006DB98. Four distinct direct jal callers are present at 0x80015EC0,
0x80015F68, 0x80016158, and 0x8006DB00. The preceding boundary is the real
jr ra/nop pair at 0x8006DB40/0x8006DB44; the following function starts at
0x8006DB9C with addu a1,zero,zero. The span is therefore a callable function,
not padding or a mislabeled tail.

## Retail semantics and screens

The function writes value0 and value1 to byte fields
D_800B0CD8[index*2+0xDC] and +0xDD, and value2 to D_800B0CD8[index+0xFE].
For index zero it ORs word flag 0x40; for index one it ORs word flag 0x80;
it returns zero in all cases.

There are no callees, no loop/back-edge, and no global other than this
word-backed overlay. The screen predicts one shared symbolic base in $v1,
scaled/index-derived addresses in $v0, a zero result accumulator, and era
-O2 -G0 compilation. The shared base is retained across byte stores and flag
loads/stores. A direct byte-global spelling was rejected as a shape probe
because it repeated symbol materialization; the union overlay is the final
minimal representation.

## Final C

```c
typedef union {
    unsigned int flags;
    unsigned char bytes[0x100];
} State;

extern State D_800B0CD8;

int func_8006DB48(int index, signed char value0, unsigned char value1,
                  unsigned char value2) {
    State *base = &D_800B0CD8;
    int result = 0;

    base->bytes[index * 2 + 0xDC] = value0;
    base->bytes[index * 2 + 0xDD] = value1;
    base->bytes[index + 0xFE] = value2;

    if (index == 0) {
        base->flags |= 0x40;
    } else if (index == 1) {
        base->flags |= 0x80;
    }

    return result;
}
```

The era compiler invocation was `-O2 -G0` with the repository's established
freestanding MIPS-1 flags. No inline asm, volatile trick, pin, or forged nop
is used.

## Object comparison

Single-leaf `mipsel-linux-gnu-objdump -dr` output:

```
00000000 <func_8006DB48>:
00000000: 3c030000  lui v1,0x0       R_MIPS_HI16 D_800B0CD8
00000004: 24630000  addiu v1,v1,0    R_MIPS_LO16 D_800B0CD8
00000008: 00041040  sll v0,a0,0x1
0000000c: 00431021  addu v0,v0,v1
00000010: a04500dc  sb a1,220(v0)
00000014: a04600dd  sb a2,221(v0)
00000018: 00831021  addu v0,a0,v1
0000001c: 14800004  bnez a0,0x30
00000020: a04700fe  sb a3,254(v0)
00000024: 8c620000  lw v0,0(v1)
00000028: 08000012  j 0x48             R_MIPS_26 .text
0000002c: 34420040  ori v0,v0,0x40
00000030: 24020001  li v0,1
00000034: 14820005  bne a0,v0,0x4c
00000038: 00000000  nop
0000003c: 8c620000  lw v0,0(v1)
00000040: 00000000  nop
00000044: 34420080  ori v0,v0,0x80
00000048: ac620000  sw v0,0(v1)
0000004c: 03e00008  jr ra
00000050: 00001021  move v0,zero
```

The 21 words after relocation normalization are:

```
3c03800b 24630cd8 00041040 00431021 a04500dc a04600dd
00831021 14800004 a04700fe 8c620000 0801b6e4 34420040
24020001 14820005 00000000 8c620000 00000000 34420080
ac620000 03e00008 00001021
```

Every non-relocatable word agrees with retail. The two symbol words and the
local jump normalize to D_800B0CD8 and 0x8006DB90; no mismatch remains.

## Carve and packed span

The former assembly segment was [0x5B1E4,0x5EFE8):

```
prefix:  0x5E348 - 0x5B1E4 = 0x3164
C leaf:  0x5E39C - 0x5E348 = 0x0054
resume:  0x5EFE8 - 0x5E39C = 0x0C4C
closure: 0x3164 + 0x0054 + 0x0C4C = 0x3E04
```

Packed-span objdump words, from two preceding boundary words through four
following words:

```
0x5E340: 0800e003 00000000 0b80033c d80c6324
0x5E350: 40100400 21104300 dc0045a0 dd0046a0
0x5E360: 21108300 04008014 fe0047a0 0000628c
0x5E370: e4b60108 40004234 01000224 05008214
0x5E380: 00000000 0000628c 00000000 80004234
0x5E390: 000062ac 0800e003 21100000 21280000
0x5E3A0: 0b80033c
```

The preceding jr ra/nop, all 21 leaf words, and the following function's
first instruction are identical to retail.

## Gates

```
candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT SHA-1 MATCH
scripts/verify_us.sh: exit 0
C conversion: Phase 5HD-12850 — 335 leaves
```

The authoritative YAML count is 335 matching-C spans; the refreshed pool has
1081 total candidates and 21 Tier-1 candidates remaining.
