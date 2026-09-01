# `func_80083C20` — exact matching-C callback initializer

Leaf 300, matched on the first bounded phrasing.

## Function hood

Retail span `[0x74420,0x7443C)`, VRAM `0x80083C20`, seven words. The body
ends in the canonical `jr ra` plus a real store delay slot. The preceding
word at file `0x7441C` is `addiu sp,sp,0x20`, the return delay slot of
`func_80083BB8`; the following word at `0x7443C` is `lbu v0,0xE9(a0)`, the
first instruction of `func_80083C3C`.

There is no direct `jal`, but `func_80083BB8` constructs the exact start at
`0x80083BF0–0x80083BF4` and stores it at offset `0x14` of its object:

```text
lui   v1,%hi(func_80083C20)
addiu v1,v1,%lo(func_80083C20)
sw    v1,0x14(s0)
```

The pool's `0/2` callers/references therefore records the HI/LO exact-start
callback-address pair, not an absence of reachability.
`FUNCTION_HOOD=PROVEN_BY_EXACT_START_CALLBACK_REFERENCE`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf callback |
| Stage-0 global readers | no global write or global access |
| coloring pressure | low: argument `$a0`, carried word `$v1`, immediate/store temp `$v0` |
| `$v0` liveness | `$v0` holds only the two byte constants; no return value |
| address retention | none; all accesses are fixed offsets from `$a0` |
| optimization signal | small immediates and the final word copy in the return delay slot support era `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_80083C20(unsigned char *a0) {
    unsigned int value = *(unsigned int *)(a0 + 0x20);
    a0[0x36] = 0x4D;
    a0[0x35] = 6;
    *(unsigned int *)(a0 + 0x2C) = value;
}
```

Compiled with `era_compile ... -O2 -G0`. No pin, inline assembly, or special
maspsx switch is used.

## Single-leaf comparison

The object has no relocations in this function.

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `8C830020` | `8C830020` | `lw v1,0x20(a0)` |
| 1 | `2402004D` | `2402004D` | `addiu v0,zero,0x4D` |
| 2 | `A0820036` | `A0820036` | `sb v0,0x36(a0)` |
| 3 | `24020006` | `24020006` | `addiu v0,zero,6` |
| 4 | `A0820035` | `A0820035` | `sb v0,0x35(a0)` |
| 5 | `03E00008` | `03E00008` | `jr ra` |
| 6 | `AC83002C` | `AC83002C` | `sw v1,0x2C(a0)` |

## Carve and packed-span proof

The original asm span is `[0x73DC0,0x74670)`, size `0x8B0`:

```text
prefix asm: 0x74420 - 0x73DC0 = 0x660
C leaf:     0x7443C - 0x74420 = 0x01C
resume asm: 0x74670 - 0x7443C = 0x234
closure:    0x660 + 0x01C + 0x234 = 0x8B0
```

Full packed span, file `0x74420..0x7443B`:

```text
retail:    2000838c4d000224360082a006000224350082a00800e0032c0083ac
candidate: 2000838c4d000224360082a006000224350082a00800e0032c0083ac
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh`
passed and reports 300 matching-C leaves.
