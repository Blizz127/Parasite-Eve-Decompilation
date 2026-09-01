# `func_80073244` — parked lexicographic compare canonicalization

`PARKED-LEXICOGRAPHIC-COMPARE-CANONICALIZATION`. Two bounded era
`-O2 -G0` source phrasings were tested. The final rejected source is
preserved in stash
`park volume func_80073244 lexicographic compare canonicalization`.

This is the third consecutive actual park and triggers the campaign hard
stop. No Tier-1 row was skipped to avoid it.

## Function hood and boundaries

Retail span `[0x63A44,0x63A94)`, VRAM `[0x80073244,0x80073294)`, is
`0x50` bytes / twenty words. It ends in canonical `jr ra; nop` at
`0x8007328C/0x80073290`.

The executable contains direct callers at `0x80073040` and `0x8007309C`.
The preceding real `func_80072F64` owns `jr ra` plus its stack-restore delay
slot at `0x8007323C/0x80073240`. The following real `func_80073294` begins
immediately by homing `$a2` and loading its fifth argument. There is no
padding ambiguity.

`FUNCTION_HOOD=PROVEN_BY_2_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Semantics and screens

The four argument registers represent two unsigned two-word values:
`first={lo:$a0,hi:$a1}` and `second={lo:$a2,hi:$a3}`. Retail performs an
unsigned lexicographic comparison, returning 1, -1, or 0.

| screen | result |
|---|---|
| callee buckets | no `jal`; true leaf |
| written-state Stage 0 | no external writes; `$a0..$a3` are homed to the caller-provided ABI stack slots because the inputs are by-value aggregates |
| coloring pressure | four argument words remain directly comparable; `$v0/$v1` carry predicates and result constants |
| `$v0` liveness | high-word predicates, low-word predicate/result, then final return |
| address retention | none; stack stores are ABI argument homes, not state |
| optimization signal | argument homing plus filled branch-delay result constants select era GCC 2.7.2 `-O2 -G0` and a by-value aggregate source type |
| loop/back-edge owner | none |

## Retail body

```text
00E5102B  sltu   v0,a3,a1
AFA40000  sw     a0,0(sp)
AFA50004  sw     a1,4(sp)
AFA60008  sw     a2,8(sp)
1440000C  bnez   v0,0x80073288
AFA7000C  sw     a3,12(sp)
00A7102B  sltu   v0,a1,a3
1440000A  bnez   v0,0x8007328C
2402FFFF  addiu  v0,zero,-1
00C4102B  sltu   v0,a2,a0
14400007  bnez   v0,0x8007328C
24020001  addiu  v0,zero,1
0086182B  sltu   v1,a0,a2
14600004  bnez   v1,0x8007328C
2402FFFF  addiu  v0,zero,-1
0801CCA3  j      0x8007328C
00001021  move   v0,zero
24020001  addiu  v0,zero,1
03E00008  jr     ra
00000000  nop
```

## Attempt 1 — scalar `unsigned long long`

```c
int func_80073244(unsigned long long first, unsigned long long second) {
    int result;

    if (first > second) result = 1;
    else if (first < second) result = -1;
    else result = 0;
    return result;
}
```

This confirms unsigned multiword comparison and has the same twenty-word
section size, but cc1 emits its generic scalar-64 comparison graph and no
argument homes. Only the first predicate and final return agree:

```text
00E5102B 14400006 00000000 14A70006
00A7102B 00C4102B 10400003 00A7102B
08000012 24020001 14400006 00000000
14E50005 00001021 0086102B 10400002
00001021 2402FFFF 03E00008 00000000
```

## Attempt 2 — two by-value word-pair aggregates

```c
typedef struct {
    unsigned int lo;
    unsigned int hi;
} WideValue;

int func_80073244(WideValue first, WideValue second) {
    int result;

    if (second.hi < first.hi) result = 1;
    else if (first.hi < second.hi) result = -1;
    else if (second.lo < first.lo) result = 1;
    else if (first.lo < second.lo) result = -1;
    else result = 0;
    return result;
}
```

This source-type lever is proven: candidate words 0–5 are retail-exact,
including the first predicate, three sequential argument-home stores, branch,
and `$a3` home in its delay slot.

After the high-word tests, cc1 canonicalizes the low-word three-way result to
one predicate followed by `negu v0,v0`. The semantic body is seventeen words;
the remaining three object words are zero assembler padding:

```text
00E5102B AFA40000 AFA50004 AFA60008
14400007 AFA7000C 00A7102B 14400007
2402FFFF 00C4102B 10400003 0086102B
0800000F 24020001 00021023 03E00008
00000000 [gas padding x3]
```

Retail preserves explicit branches and result constants for both low-word
directions; era cc1 recognizes and folds that equivalent tail. The by-value
aggregate recovers the ABI shape but cannot disable this comparison-result
canonicalization. A third equivalent spelling would violate the bounded
phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 334, and consecutive parks become three.

`HARD_STOP=THREE_CONSECUTIVE_ACTUAL_PARKS`
