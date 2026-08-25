# `func_80083E50` — exact matching-C object initializer

Leaf 305, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x74650,0x74670)`, VRAM `0x80083E50`, eight words. It ends in
`jr ra` with a real byte store in the delay slot. The preceding word at
`0x7464C` is the real return delay slot of `func_80083DF0`; the following word
at `0x74670` is the real `addiu v0,zero,0x45` start of `func_80083E70`.

Two exact direct callers occur at `0x80084BF4` and `0x80084C04`.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf initializer |
| global access | none |
| coloring pressure | low: `$v0` is reused for constants and the interior pointer |
| `$v0` liveness | `0x43`, then `a0+0x24`, then state value `1`; no return value |
| address retention | only the argument-relative interior pointer stored at `a0+0x2C` |
| optimization signal | reused `$v0` and final store in the return delay slot select era `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_80083E50(unsigned char *a0, unsigned char a1) {
    a0[0x36] = 0x43;
    *(unsigned char **)(a0 + 0x2C) = a0 + 0x24;
    a0[0x24] = a1;
    a0[0x35] = 1;
}
```

Compiled with `era_compile ... -O2 -G0`. There are no relocations, pins,
inline assembly, or special maspsx switches.

## Full single-leaf comparison

| word | retail | candidate | instruction |
|---:|---:|---:|---|
| 0 | `24020043` | `24020043` | `addiu v0,zero,0x43` |
| 1 | `A0820036` | `A0820036` | `sb v0,0x36(a0)` |
| 2 | `24820024` | `24820024` | `addiu v0,a0,0x24` |
| 3 | `AC82002C` | `AC82002C` | `sw v0,0x2C(a0)` |
| 4 | `24020001` | `24020001` | `addiu v0,zero,1` |
| 5 | `A0850024` | `A0850024` | `sb a1,0x24(a0)` |
| 6 | `03E00008` | `03E00008` | `jr ra` |
| 7 | `A0820035` | `A0820035` | `sb v0,0x35(a0)` |

## Carve and packed-span proof

The prior asm span is `[0x7443C,0x74670)`, size `0x234`:

```text
prefix asm: 0x74650 - 0x7443C = 0x214
C leaf:     0x74670 - 0x74650 = 0x020
closure:    0x214 + 0x020 = 0x234
```

Full packed span, file `0x74650..0x7466F`:

```text
retail:    43000224360082a0240082242c0082ac01000224240085a00800e003350082a0
candidate: 43000224360082a0240082242c0082ac01000224240085a00800e003350082a0
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh`
passed and reports 305 matching-C leaves.
