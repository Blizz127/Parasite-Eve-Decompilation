# `func_80077AC4` — parked mask-constant register coloring

`PARKED-MASK-CONSTANT-COLORING`. Two bounded era `-O2 -G0` source
phrasings were tested. The rejected source is preserved in stash
`park volume func_80077AC4 mask constant coloring`.

## Function hood and boundaries

Retail span `[0x682C4,0x68300)`, VRAM `[0x80077AC4,0x80077B00)`, is
`0x3C` bytes / fifteen words. It ends in canonical `jr ra` with the final
word store in the return delay slot.

The executable contains 55 direct `jal 0x80077AC4` references:

```text
0x8002B92C 0x80031648 0x80031AD8 0x80031CE4 0x80031D10
0x80031D4C 0x80031E30 0x80032004 0x800321B4 0x80032370
0x800325C4 0x800326AC 0x80032858 0x80032AC0 0x80032D70
0x80033308 0x80033484 0x80033598 0x80033628 0x800336B8
0x80033748 0x800337B8 0x80033828 0x80033898 0x80033908
0x80033974 0x80033CB8 0x80033CEC 0x80033D20 0x80033D54
0x80033EE4 0x80033F18 0x80033F4C 0x800340B8 0x8003441C
0x80034714 0x80034780 0x800378EC 0x80037B48 0x80037DF0
0x80037F38 0x800381B4 0x80038328 0x80038468 0x800385A8
0x800386E8 0x80038864 0x800D1FAC 0x800D27C0 0x800D2A40
0x800D30E0 0x800D372C 0x800D38E8 0x800D3AA0 0x800D3F2C
```

Matched `func_80077AA4` ends at `0x80077AB4/0x80077AB8` with `jr ra` and
its live `andi` delay slot; two alignment nops at `0x80077ABC/0x80077AC0`
precede this function. This function ends at `0x80077AFC`; one alignment
nop at `0x80077B00` precedes real `func_80077B04`, whose first word is a
branch. Boundary ownership is unambiguous.

`FUNCTION_HOOD=PROVEN_BY_55_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; two-word link helper |
| written-state Stage 0 | argument-relative only: rewrites words at both pointer arguments; no global writer |
| coloring pressure | two pointers, two 32-bit mask constants, and two loaded values remain live; retail assigns low-24 mask to `$a2` and high-byte mask to `$a3` |
| `$v0` liveness | loads `*first` twice, carries its masked high byte, then carries the final second store value |
| address retention | both pointer arguments remain live; `$a1` is finally converted in place to its low-24 address |
| optimization signal | constant hoisting, memory schedule, and return-delay store select era GCC 2.7.2 `-O2 -G0` |
| loop/back-edge owner | none |

Existing retail-text cross-reference evidence identifies this as an ordering
table insertion helper. The machine-level semantics exchange low-24 link
information while preserving each word's high byte; no additional Psy-Q
routine-name claim is needed for the match attempt.

## Retail body

```text
3C0600FF  lui   a2,0x00FF
34C6FFFF  ori   a2,a2,0xFFFF
3C07FF00  lui   a3,0xFF00
8CA30000  lw    v1,0(a1)
8C820000  lw    v0,0(a0)
00671824  and   v1,v1,a3
00461024  and   v0,v0,a2
00621825  or    v1,v1,v0
ACA30000  sw    v1,0(a1)
8C820000  lw    v0,0(a0)
00A62824  and   a1,a1,a2
00471024  and   v0,v0,a3
00451025  or    v0,v0,a1
03E00008  jr    ra
AC820000  sw    v0,0(a0)
```

## Attempt 1 — direct link expressions

```c
void func_80077AC4(unsigned int *first, unsigned int *second) {
    *second = (*second & 0xFF000000) | (*first & 0x00FFFFFF);
    *first = (*first & 0xFF000000) |
             ((unsigned int)second & 0x00FFFFFF);
}
```

This produces the exact fifteen-word shape, exact load/store schedule,
exact dataflow, and exact return-delay store. Eight words match. The sole
mechanism is that GCC assigns the constants to the opposite argument
registers: candidate `$a3=0x00FFFFFF/$a2=0xFF000000`, retail
`$a2=0x00FFFFFF/$a3=0xFF000000`.

```text
3C0700FF 34E7FFFF 3C06FF00 8CA30000
8C820000 00661824 00471024 00621825
ACA30000 8C820000 00A72824 00461024
00451025 03E00008 AC820000
```

## Attempt 2 — explicit loaded values and reversed OR terms

```c
void func_80077AC4(unsigned int *first, unsigned int *second) {
    unsigned int first_value = *first;
    unsigned int second_value = *second;

    *second = (first_value & 0x00FFFFFF) |
              (second_value & 0xFF000000);
    *first = (*first & 0xFF000000) |
             ((unsigned int)second & 0x00FFFFFF);
}
```

The mask registers remain swapped, and the explicit value ordering also
swaps the first two loads. It remains fifteen words and 8/15 exact:

```text
3C0700FF 34E7FFFF 3C06FF00 8C830000
8CA20000 00671824 00461024 00621825
ACA30000 8C820000 00A72824 00461024
00451025 03E00008 AC820000
```

The semantic operations, constants, addresses, size, and ending all agree.
Closing this requires a proven register-coloring lever for the two hoisted
mask constants; a third equivalent spelling would violate the bounded
phrasing rule.

No YAML/build/verifier integration was made. The accepted executable remains
exact, matching-C stays 329, and consecutive parks become two.
