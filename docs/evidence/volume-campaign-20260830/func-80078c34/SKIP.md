# `func_80078C34` — handwritten libGTE/COP2 screen

Disposition: `SKIP-SDK-LIBRARY-COP2`. No ordinary-C attempt, carve,
integration, stash, or matching-C count change was made.

## Function hood and boundaries

Retail span `[0x69434,0x69490)`, VRAM `[0x80078C34,0x80078C90)`, is 0x5C
bytes / 23 words. It ends in canonical `jr ra; nop` at
`0x80078C88/0x80078C8C`.

The preceding real handwritten function ends at `0x80078C2C/0x80078C30`
with its own `jr ra; nop`. The word immediately after this span,
`0x80078C90`, is alignment padding; real `func_80078C94` begins at
`0x80078C94` with `lw t0,0(a1)`. Both functional boundaries are explicit.

The retail executable has 25 direct `jal 0x80078C34` callers:

```text
0x800C3EFC 0x800C3F18 0x800C3F34 0x800C3F50 0x800C5648
0x800C5658 0x800C61D4 0x800C61E4 0x800C61F4 0x800C6204
0x800C7B44 0x800C7F08 0x800C7FAC 0x800C80B0 0x800C81A8
0x800C904C 0x800C90F0 0x800C91F4 0x800C9D44 0x800C9DE8
0x800CA8DC 0x800CA980 0x800CAAB8 0x800CAB18 0x800CAC14
```

`FUNCTION_HOOD=PROVEN_BY_25_DIRECT_CALLERS_AND_CANONICAL_RETURN`.

## Retail body and screens

The generated split explicitly marks this entry `Handwritten function`:

```text
69434  lw    t0,0(a0)
69438  lw    t1,4(a0)
6943C  lw    t2,8(a0)
69440  lw    t3,12(a0)
69444  lw    t4,16(a0)
69448  ctc2  t0,$0
6944C  ctc2  t1,$1
69450  ctc2  t2,$2
69454  ctc2  t3,$3
69458  ctc2  t4,$4
6945C  lwc2  $0,0(a1)
69460  lwc2  $1,4(a1)
69464  nop
69468  mvmva 1,0,0,3,0
6946C  mfc2  t0,$9
69470  mfc2  t1,$10
69474  mfc2  t2,$11
69478  sh    t0,0(a2)
6947C  sh    t1,2(a2)
69480  sh    t2,4(a2)
69484  move  v0,a2
69488  jr    ra
6948C  nop
```

| screen | result |
|---|---|
| callee buckets | no `jal`; architectural GTE operations are the body |
| written-global Stage 0 | none; only the caller-supplied result at `a2[0..5]` is written |
| coloring pressure | five matrix words occupy `t0..t4`; three GTE results return through `t0..t2` |
| `$v0` liveness | result pointer is copied from `a2` only after all GTE stores |
| address retention | `a0` is matrix/control source, `a1` vector source, `a2` output/result |
| optimization signal | none: handwritten marker plus COP2 transfers, not cc1 codegen |
| loop/back-edge owner | none |

The first five words load a matrix/control block, `ctc2` installs it in GTE
control registers 0--4, `lwc2` loads the input vector, `mvmva` performs the
matrix/vector operation, and `mfc2` stores the three 16-bit results. This is
proven libGTE support by instruction role and adjacency. The exact PsyQ routine
name is intentionally not assigned without symbol/string provenance.

The existing policy in `docs/ai_context/sdk_map.md` redirects libGTE to
PsyCross and does not schedule handwritten SDK code as PE1 game-logic C. Its
COP2 instructions also have no sanctioned ordinary-C intrinsic. Inline asm is
forbidden. Therefore the correct campaign action is a zero-attempt SDK screen,
not a park and not matching-C progress.

`MATCHING_C_COUNT=335`

`INTEGRATION=NONE`
