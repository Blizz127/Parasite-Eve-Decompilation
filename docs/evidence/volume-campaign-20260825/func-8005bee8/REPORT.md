# `func_8005BEE8` — exact matching-C selected-pointer getter

Leaf 303, matched on the first bounded phrasing.

## Function hood and boundaries

Retail span `[0x4C6E8,0x4C708)`, VRAM `0x8005BEE8`, eight words. It ends in
`jr ra` with `move v0,v1` in the delay slot. The preceding word at `0x4C6E4`
is the real return delay slot of `func_8005BEDC`; the following word at
`0x4C708` is the real `lw v1,0x354(gp)` start of `func_8005BF08`.

Six exact direct callers occur at `0x80043B38`, `0x8004DFF4`, `0x8004E004`,
`0x8004E284`, `0x8004E420`, and `0x8004E6A0`.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail screens

| screen | result |
|---|---|
| callee buckets | no `jal`; leaf getter |
| global access | reads gp-relative selector `D_8009D218`; writes no global |
| coloring pressure | `$v0` holds the selector while `$v1` retains the candidate address |
| `$v0` liveness | selector until the return delay slot, then result pointer |
| address retention | explicit result pointer remains in `$v1` across the conditional increment |
| optimization signal | gp-relative selector plus era-style address materialization select `-O2 -G8` |
| loop/back-edge owner | none |

The selector has four other static readers (`func_80054A88`,
`func_8005BCB0`, `func_8005BCBC`, `func_8005C374`) and an existing setter,
`func_8005BC98`. No higher-level name is assigned from this alone.

## C and flags

```c
extern int D_8009D218;
extern unsigned char D_800C0DE0[];

unsigned char *func_8005BEE8(void) {
    int select = D_8009D218;
    unsigned char *result = D_800C0DE0;

    if (select != 0) {
        result += 0x10;
    }
    return result;
}
```

Compiled with `era_compile ... -O2 -G8`. The object's GPREL16 relocation for
`D_8009D218` resolves to `0x4A8`; its HI16/LO16 pair for `D_800C0DE0`
resolves to `0x800C/0x0DE0`. No pin, inline assembly, or special maspsx
switch is used.

## Full single-leaf comparison

| word | retail | candidate after relocation | instruction |
|---:|---:|---:|---|
| 0 | `8F8204A8` | `8F8204A8` | `lw v0,0x4A8(gp)` |
| 1 | `3C03800C` | `3C03800C` | `lui v1,%hi(D_800C0DE0)` |
| 2 | `24630DE0` | `24630DE0` | `addiu v1,v1,%lo(D_800C0DE0)` |
| 3 | `10400002` | `10400002` | `beqz v0,+2` |
| 4 | `00000000` | `00000000` | `nop` |
| 5 | `24630010` | `24630010` | `addiu v1,v1,0x10` |
| 6 | `03E00008` | `03E00008` | `jr ra` |
| 7 | `00601021` | `00601021` | `addu v0,v1,zero` |

## Carve and packed-span proof

The prior asm span is `[0x4C6E8,0x4CC88)`, size `0x5A0`:

```text
C leaf:     0x4C708 - 0x4C6E8 = 0x020
resume asm: 0x4CC88 - 0x4C708 = 0x580
closure:    0x020 + 0x580 = 0x5A0
```

Full packed span, file `0x4C6E8..0x4C707`:

```text
retail:    a804828f0c80033ce00d63240200401000000000100063240800e00321106000
candidate: a804828f0c80033ce00d63240200401000000000100063240800e00321106000
```

The packed executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. `scripts/verify_us.sh`
passed and reports 303 matching-C leaves.
