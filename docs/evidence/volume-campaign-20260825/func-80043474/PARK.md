# `func_80043474` — parked threshold-ladder block layout

`PARKED-THRESHOLD-LADDER-BLOCK-LAYOUT`. Two bounded era `-O2 -G0` source
phrasings were tested. The final rejected source is preserved in stash
`park volume func_80043474 threshold ladder block layout`.

## Function hood and boundaries

Retail span `[0x33C74,0x33CC0)`, VRAM `[0x80043474,0x800434C0)`, is
`0x4C` bytes / nineteen words. It ends in canonical `jr ra` with the result
copy in the return delay slot.

A raw executable scan finds direct callers at `0x800402F4` and
`0x8005C340`. The preceding real `func_8004324C` owns `jr ra; nop` at
`0x8004346C/0x80043470`; the following real `func_800434C0` begins
immediately with a gp-relative load. There is no padding ambiguity.

`FUNCTION_HOOD=PROVEN_BY_2_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Semantics and screens

The signed threshold ladder returns:

```text
value < 0x080  -> 1
value < 0x138  -> 2
value < 0x1B0  -> 3
value < 0x218  -> 4
value < 0x2D3  -> 5
otherwise      -> 6
```

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | none; pure integer classifier |
| coloring pressure | `$v0` carries each signed predicate; `$v1` is the explicit result accumulator |
| `$v0` liveness | five independent `slti` predicates, then final result copy from `$v1` |
| address retention | none |
| optimization signal | constants in branch delay slots and a live return-delay result copy select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

## Retail body

```text
28820080  slti   v0,a0,0x80
1440000F  bnez   v0,0x800434B8
24030001  addiu  v1,zero,1
28820138  slti   v0,a0,0x138
1440000C  bnez   v0,0x800434B8
24030002  addiu  v1,zero,2
288201B0  slti   v0,a0,0x1B0
14400009  bnez   v0,0x800434B8
24030003  addiu  v1,zero,3
28820218  slti   v0,a0,0x218
14400005  bnez   v0,0x800434B4
288202D3  slti   v0,a0,0x2D3
10400004  beqz   v0,0x800434B8
24030006  addiu  v1,zero,6
08010D2E  j      0x800434B8
24030005  addiu  v1,zero,5
24030004  addiu  v1,zero,4
03E00008  jr     ra
00601021  move   v0,v1
```

The fourth branch is the distinguishing structure: its delay slot computes
the fifth predicate even though the taken category-4 arm does not consume
it. Categories 5 and 6 then select through a branch and local jump before the
separate category-4 assignment block.

## Attempt 1 — ascending less-than ladder

```c
int func_80043474(int value) {
    int result;

    if (value < 0x80) result = 1;
    else if (value < 0x138) result = 2;
    else if (value < 0x1B0) result = 3;
    else if (value < 0x218) result = 4;
    else if (value < 0x2D3) result = 5;
    else result = 6;

    return result;
}
```

The first three compare/result pairs match in opcode and register homes, but
their branch distances reflect an extra word later. For category 4, cc1 uses
`beqz; nop; j; li 4`, then computes the fifth predicate. The candidate is
twenty words:

```text
28820080 14400010 24030001 28820138
1440000D 24030002 288201B0 1440000A
24030003 28820218 10400003 00000000
08000012 24030004 288202D3 10400002
24030006 24030005 03E00008 00601021
```

## Attempt 2 — complementary final condition

The final two arms were respelled to test CFG polarity:

```c
    else if (value < 0x218) result = 4;
    else if (value >= 0x2D3) result = 6;
    else result = 5;
```

This flips only the category-5/6 branch. cc1 retains the same separate
category-4 `beqz` plus jump and the same twenty-word size:

```text
28820080 14400010 24030001 28820138
1440000D 24030002 288201B0 1440000A
24030003 28820218 10400003 00000000
08000012 24030004 288202D3 14400002
24030005 24030006 03E00008 00601021
```

Retail's one-word advantage is a cross-block scheduling/layout decision:
hoist the fifth compare into the category-4 branch delay slot and place the
category-4 assignment after the category-5/6 jump. Neither semantic condition
polarity selects that layout in era cc1. A third equivalent spelling would
violate the bounded phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 334, and consecutive parks become two.
