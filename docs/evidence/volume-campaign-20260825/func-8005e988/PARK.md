# `func_8005E988` — parked control-flow constant scheduling

`PARKED-CONTROL-FLOW-CONSTANT-SCHEDULING`. Two bounded era `-O2 -G8`
source phrasings were tested. The final rejected source is preserved in stash
`park volume func_8005E988 control-flow constant scheduling`.

## Function hood and boundaries

Retail span `[0x4F188,0x4F1C8)`, VRAM `[0x8005E988,0x8005E9C8)`, is
`0x40` bytes / sixteen words. It ends in canonical `jr ra; nop` at
`0x8005E9C0/0x8005E9C4`.

The executable contains twelve direct `jal 0x8005E988` references:

```text
0x80045770 0x800457A4 0x800457CC 0x80045828
0x800458B0 0x8004592C 0x800459B4 0x800459FC
0x80045A44 0x8004A414 0x8004A490 0x8004A50C
```

The preceding real function, matched `func_8005E968`, owns its canonical
return at `0x8005E980/0x8005E984`. The following real function begins at
`0x8005E9C8` with `lw v0,0x398(gp)`. Both boundary sides are executable
instructions, and there is no padding ambiguity.

`FUNCTION_HOOD=PROVEN_BY_12_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf selector/writer |
| written-state Stage 0 | writes gp-relative `D_8009D110` and `D_8009D114`; accepted sibling `func_8005E968` independently proves the same paired 32-bit state |
| coloring pressure | `$v1` holds the selected 24-bit constant while `$v0` is first a compare predicate and then the shifted second output |
| `$v0` liveness | two signed-compare predicates, then `value >> 1` for the second store |
| address retention | none; both stores are ordinary gp-relative accesses |
| optimization signal | three split 24-bit constants are built around branch and jump delay slots; gp accesses select era `-O2 -G8` |
| loop/back-edge owner | none |

The machine-level semantics select `0x408080` when `first < second`,
`0x404080` when `second < first`, and `0x808080` when equal. The selected
value and its unsigned half are stored to the proven global pair. The packed
values resemble color data, but no stronger semantic name is asserted here.

## Retail body

```text
0085102A  slt   v0,a0,a1
14400008  bnez  v0,0x8005E9B0
3C030040  lui   v1,0x0040
3C030080  lui   v1,0x0080
00A4102A  slt   v0,a1,a0
10400005  beqz  v0,0x8005E9B4
34638080  ori   v1,v1,0x8080
3C030040  lui   v1,0x0040
08017A6D  j     0x8005E9B4
34634080  ori   v1,v1,0x4080
34638080  ori   v1,v1,0x8080
00031042  srl   v0,v1,1
AF8303A0  sw    v1,0x3A0(gp)
AF8203A4  sw    v0,0x3A4(gp)
03E00008  jr    ra
00000000  nop
```

With `_gp = 0x8009CD70`, the stores normalize to
`D_8009D110-_gp = 0x3A0` and `D_8009D114-_gp = 0x3A4`.

## Attempt 1 — natural nested selection

```c
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;

void func_8005E988(int first, int second) {
    unsigned int value;

    if (first < second) {
        value = 0x408080;
    } else if (second < first) {
        value = 0x404080;
    } else {
        value = 0x808080;
    }

    D_8009D110 = value;
    D_8009D114 = value >> 1;
}
```

The candidate has the correct sixteen-word size, values, final shift, stores,
and return. It diverges at word 1: GCC selects a `beqz` topology and jumps
around the remaining selection for the first arm.

```text
0085102A 10400003 3C030040 0800000B
34638080 3C030080 00A4102A 10400003
34638080 3C030040 34634080 00031042
AF830000 AF820000 03E00008 00000000
```

The two `AF8x0000` words carry ordinary `R_MIPS_GPREL16` relocations for
`D_8009D110` and `D_8009D114`; normalization changes only their immediate
fields and does not resolve the control-flow mismatch.

## Attempt 2 — explicit less-case default

```c
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;

void func_8005E988(int first, int second) {
    unsigned int value;

    value = 0x408080;
    if (first >= second) {
        value = 0x808080;
        if (second < first) {
            value = 0x404080;
        }
    }

    D_8009D110 = value;
    D_8009D114 = value >> 1;
}
```

This phrasing selects the retail first `bnez`, but GCC hoists the first
`lui v1,0x40` ahead of the initial comparison. It consequently has fifteen
content words; the sixteenth object word is assembler padding, not function
code.

```text
3C030040 0085102A 14400007 34638080
3C030080 00A4102A 10400003 34638080
3C030040 34634080 00031042 AF830000
AF820000 03E00008 00000000 [gas padding]
```

Retail deliberately intertwines the three constant constructions with two
branch delay slots and one local-jump delay slot. Natural source chooses an
early local jump; explicit default source hoists a constant and shrinks the
body. The semantic operation, signed comparisons, constants, gp destinations,
and final stores are proven; the remaining difference is compiler block
layout and constant-materialization scheduling. A third equivalent spelling
would violate the bounded phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 332, and consecutive parks become one.
