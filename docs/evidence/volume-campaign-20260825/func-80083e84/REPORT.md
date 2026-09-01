# `func_80083E84` — exact matching-C initializer twin

Leaf 306, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x74684,0x746A4)`, VRAM `0x80083E84`, eight words. It ends in
`jr ra` with a real byte store in the delay slot. The preceding word at
`0x74680` is the real return delay slot of `func_80083E70`; the following word
at `0x746A4` is the real first instruction of `func_80083EA4`.

Two exact direct callers occur at `0x80083618` and `0x800838E4`.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf initializer |
| global access | none |
| coloring pressure | low: `$v0` is reused for tag, interior pointer, and state value |
| `$v0` liveness | `0x4C`, then `a0+0x24`, then `1`; no return value |
| address retention | only the argument-relative interior pointer stored at `a0+0x2C` |
| optimization signal | exact structural twin of `func_80083E50`; era `-O2 -G0` |
| loop/back-edge owner | none |

## C and flags

```c
void func_80083E84(unsigned char *a0, unsigned char a1) {
    a0[0x36] = 0x4C;
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
| 0 | `2402004C` | `2402004C` | `addiu v0,zero,0x4C` |
| 1 | `A0820036` | `A0820036` | `sb v0,0x36(a0)` |
| 2 | `24820024` | `24820024` | `addiu v0,a0,0x24` |
| 3 | `AC82002C` | `AC82002C` | `sw v0,0x2C(a0)` |
| 4 | `24020001` | `24020001` | `addiu v0,zero,1` |
| 5 | `A0850024` | `A0850024` | `sb a1,0x24(a0)` |
| 6 | `03E00008` | `03E00008` | `jr ra` |
| 7 | `A0820035` | `A0820035` | `sb v0,0x35(a0)` |

## Carve and packed-span proof

The prior asm span is `[0x74684,0x746E4)`, size `0x60`:

```text
C leaf:     0x746A4 - 0x74684 = 0x20
resume asm: 0x746E4 - 0x746A4 = 0x40
closure:    0x20 + 0x40 = 0x60
```

Full packed span, file `0x74684..0x746A3`:

```text
retail:    4c000224360082a0240082242c0082ac01000224240085a00800e003350082a0
candidate: 4c000224360082a0240082242c0082ac01000224240085a00800e003350082a0
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. The first verifier run rejected
the stale expected marker `[0x74684, asm]`; its manifest was corrected to the
actual C/asm geometry (also banking the earlier `83C20`/`83E50` markers).
The second `scripts/verify_us.sh` run passed and reports 306 matching-C leaves.
