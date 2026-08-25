# `func_800792D4` — screened handwritten libGTE/COP2 operation

`SKIP-SDK-LIBRARY-COP2`. No C attempt was made and the matching-C count
remains 309.

## Function hood and boundaries

Retail span `[0x69AD4,0x69AFC)`, VRAM `0x800792D4`, ten words. It ends in
`jr ra` with a real output store in the delay slot. Six exact direct callers
occur at:

```text
0x80031A40  0x80031A50  0x80031A60
0x800C4540  0x800C4C38  0x800C5250
```

The preceding real function returns at `0x69AC0`/`0x69AC4`, followed by three
alignment nops. Two alignment nops after this span precede the next real
function at `0x69B04`. The padding is outside the exact-start called span.
`FUNCTION_HOOD=PROVEN_BY_DIRECT_CALLERS`.

## Retail body

```text
C8800000  lwc2  $0,0(a0)
C8810004  lwc2  $1,4(a0)
00000000  nop
4A480012  mvmva 1,0,0,0,0
E8B90000  swc2  $25,0(a1)
E8BA0004  swc2  $26,4(a1)
E8BB0008  swc2  $27,8(a1)
4842F800  cfc2  v0,$31
03E00008  jr    ra
ACC20000  sw    v0,0(a2)
```

## Screen and disposition

| screen | result |
|---|---|
| callee buckets | no `jal`; one direct GTE operation |
| globals / Stage 0 | none |
| semantic side effects | COP2 data loads, `mvmva`, COP2 result stores, and flag-register read |
| ordinary-C expressibility | absent in the sanctioned subset; would require intrinsics or assembly |
| provenance | handwritten annotation inside the established libGTE region of `asm/disc1/68478.s`; callers use it beside matrix/COP2 setup helpers |
| exact SDK name | not proven and not assigned |

This is a real callable SDK helper, not padding and not a compiler blocker.
The project redirects libGTE to PsyCross for the port; literal inline or
file-scope assembly is forbidden as matching-C progress. It is therefore
removed from the matching campaign without consuming an attempt.
