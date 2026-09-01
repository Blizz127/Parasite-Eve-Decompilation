# `func_80082ADC` — parked symbolic-base retention

No integration and no matching-count change. Two bounded natural-C phrasings
were attempted; GCC 2.7.2 lowers both to four independent symbolic stores,
while retail retains one symbolic base across all four writes.

`DISPOSITION=PARKED-SYMBOLIC-BASE-RETENTION`.

## Function hood and boundaries

Retail span `[0x732DC,0x73308)`, VRAM `0x80082ADC`, is eleven words and ends
in canonical `jr ra` plus the final zero store in its delay slot. One exact
direct call targets its start:

```text
file 0x74E18 / VA 0x80084618: jal func_80082ADC
```

The preceding real `func_800829C4` ends at `0x80082AD4/0x80082AD8` with
`jr ra` and stack restoration. The following real `func_80082B08` starts at
`0x80082B08` with a global load and then establishes a stack frame. Both
boundary words are executable instructions; no padding belongs to this span.

`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLER`.

## Screens and state provenance

| screen | result |
|---|---|
| callee buckets | no `jal`; callback-record initializer |
| written-state Stage 0 | writes callbacks at `D_800A5AB4/+4` and clears the surrounding words at `D_800A5AB0/D_800A5ABC`; the containing record at `D_800A5AB0` is subsequently passed to `func_8007E1F4` and `func_8007E1E4` by the setup path at `0x80082D0C` |
| coloring pressure | retail retains the record base in `$v0` and uses `$v1` for each callback address |
| `$v0` liveness | symbolic base is live from entry through the return-delay-slot store |
| address retention | four stores share one retained `D_800A5AB4` base, including offsets `-4`, `0`, `4`, and `8` |
| optimization signal | Psy-Q-era `lui/addiu` address materialization and scheduling select era `-O2 -G0` |
| loop/back-edge owner | none |

The exact consumer path constructs `D_800A5AB0` at `0x80082D0C`, passes it
as argument 1 to `func_8007E1F4(2, record)` at `0x80082D14`, then to
`func_8007E1E4(2, record)` at `0x80082D20`. Those wrappers are the existing
kernel interrupt-record enqueue/dequeue path; this report does not infer a
more specific source name for the callbacks.

## Retail, all eleven words

```text
80082ADC: 3C02800A  lui    v0,%hi(D_800A5AB4)
80082AE0: 24425AB4  addiu  v0,v0,%lo(D_800A5AB4)
80082AE4: 3C038008  lui    v1,%hi(func_80082B70)
80082AE8: 24632B70  addiu  v1,v1,%lo(func_80082B70)
80082AEC: AC430000  sw     v1,0(v0)
80082AF0: 3C038008  lui    v1,%hi(func_80082B08)
80082AF4: 24632B08  addiu  v1,v1,%lo(func_80082B08)
80082AF8: AC430004  sw     v1,4(v0)
80082AFC: AC40FFFC  sw     zero,-4(v0)
80082B00: 03E00008  jr     ra
80082B04: AC400008  sw     zero,8(v0)
```

## Attempt 1 — direct callback-array stores

```c
typedef void (*Callback)(void);

extern Callback D_800A5AB4[];
extern void func_80082B08(void);
extern void func_80082B70(void);

void func_80082ADC(void) {
    D_800A5AB4[0] = func_80082B70;
    D_800A5AB4[1] = func_80082B08;
    D_800A5AB4[-1] = 0;
    D_800A5AB4[2] = 0;
}
```

Era GCC 2.7.2-psx plus maspsx 2.21, `-O2 -G0`, emits fourteen words after
relocation:

```text
00000000: 3C028008  lui    v0,%hi(func_80082B70)
00000004: 24422B70  addiu  v0,v0,%lo(func_80082B70)
00000008: 3C01800A  lui    at,%hi(D_800A5AB4)
0000000C: AC225AB4  sw     v0,%lo(D_800A5AB4)(at)
00000010: 3C028008  lui    v0,%hi(func_80082B08)
00000014: 24422B08  addiu  v0,v0,%lo(func_80082B08)
00000018: 3C01800A  lui    at,%hi(D_800A5AB4)
0000001C: AC225AB8  sw     v0,%lo(D_800A5AB4+4)(at)
00000020: 3C01800A  lui    at,%hi(D_800A5AB4)
00000024: AC205AB0  sw     zero,%lo(D_800A5AB4-4)(at)
00000028: 3C01800A  lui    at,%hi(D_800A5AB4)
0000002C: AC205ABC  sw     zero,%lo(D_800A5AB4+8)(at)
00000030: 03E00008  jr     ra
00000034: 00000000  nop
```

The first mismatch is word 0: candidate materializes the first callback in
`$v0`, while retail first establishes and retains the record base in `$v0`.

## Attempt 2 — explicit common base

The bounded retry introduced an explicit local pointer and performed the same
four stores through it:

```c
void func_80082ADC(void) {
    Callback *record = D_800A5AB4;

    record[0] = func_80082B70;
    record[1] = func_80082B08;
    record[-1] = 0;
    record[2] = 0;
}
```

GCC folds the local away before assembly and emits exactly the same symbolic
store sequence and fourteen-word object as attempt 1. The source-level common
base therefore does not reach the assembler; changing maspsx globally on this
evidence would risk the already-proven `$at` symbolic-store sites.

## Disposition

This is a new multi-store member of the banked address-retention pressure
class. It is neither padding nor compiler skew: ordinary source expresses the
semantics, but the era compiler does not retain the symbolic base register
that retail uses. No pin, inline assembly, or assembler patch was attempted.
Revisit only with a proven source idiom or a discriminating toolchain rule.

The final candidate source is preserved in stash
`park func_80082ADC symbolic-base retention attempts`. Matching C remains
320, and this is consecutive park 1.
