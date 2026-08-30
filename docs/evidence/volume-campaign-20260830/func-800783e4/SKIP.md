# `func_800783E4` — handwritten libGTE/COP2 screen

Disposition: `SKIP-SDK-LIBRARY-COP2`. No ordinary-C phrasing, source file,
carve, integration, stash, or matching-C count change was made.

## Function hood

- Retail file span `[0x68BE4,0x68C6C)` / VRAM
  `[0x800783E4,0x8007846C)` is `0x88` bytes / 34 words.
- It ends in canonical `jr ra; nop` at `0x80078464/0x80078468`.
- The preceding real handwritten function, `func_80078394`, ends immediately
  at `0x800783DC/0x800783E0` with its own `jr ra; nop`.
- The following real handwritten function, `func_8007846C`, begins
  immediately at `0x8007846C` with `lw t0,0(a0)`.
- A raw executable search for little-endian `jal 0x800783E4` word
  `F9 E0 01 0C` finds exactly three direct callers:

| caller file PC | caller VA |
|---:|---:|
| `0xC46BC` | `0x800D3EBC` |
| `0xCDC34` | `0x800DD434` |
| `0xCE298` | `0x800DDA98` |

`FUNCTION_HOOD=PROVEN_BY_THREE_DIRECT_CALLERS_AND_CANONICAL_RETURN`. This is
a genuine callable function, not padding or a generated-label inference.

## Retail body and screens

The generated split explicitly marks the entry `Handwritten function`:

```text
68BE4  lw    t0,0(a0)
68BE8  lw    t2,4(a0)
68BEC  sra   t1,t0,16
68BF0  andi  t0,t0,0xFFFF
68BF4  andi  t2,t2,0xFFFF
68BF8  mtc2  a2,$8
68BFC  mtc2  t0,$9
68C00  mtc2  t1,$10
68C04  mtc2  t2,$11
68C08  nop
68C0C  gpf   1
68C10  lw    t0,0(a1)
68C14  lw    t2,4(a1)
68C18  sra   t1,t0,16
68C1C  andi  t0,t0,0xFFFF
68C20  andi  t2,t2,0xFFFF
68C24  mfc2  v0,$31
68C28  mtc2  a3,$8
68C2C  mtc2  t0,$9
68C30  mtc2  t1,$10
68C34  mtc2  t2,$11
68C38  nop
68C3C  gpl   1
68C40  mfc2  t0,$9
68C44  mfc2  t1,$10
68C48  andi  t0,t0,0xFFFF
68C4C  sll   t1,t1,16
68C50  or    t0,t0,t1
68C54  lw    t5,0x10(sp)
68C58  mfc2  t2,$11
68C5C  sw    t0,0(t5)
68C60  sw    t2,4(t5)
68C64  jr    ra
68C68  nop
```

| screen | result |
|---|---|
| callee buckets | no `jal`; GTE data transfers and arithmetic are the semantic body |
| Stage-0 written globals | none; only the caller's fifth-argument output pointer is written |
| coloring pressure | packed input components occupy `t0/t1/t2`; output words return through `t0/t1/t2`; fifth argument remains at `0x10(sp)` |
| `$v0` liveness | the unused `mfc2 v0,$31` is an architectural sequencing read, not a C return value |
| address retention | `a0` and `a1` are packed input sources; output pointer is reloaded from the o32 fifth-argument home |
| optimization signal | none; explicit handwritten marker and architectural COP2 sequencing dominate |
| loop/back-edge owner | none |

The function installs packed components in GTE data registers 8--11,
executes `gpf`, installs the second packed input, executes `gpl`, and writes
the resulting packed values. Ordinary C has no sanctioned expression for
these COP2 side effects. Inline assembly and intrinsic invention are outside
the campaign rules.

The neighboring handwritten GTE cluster and the established policy in
`docs/ai_context/sdk_map.md` route libGTE support to PsyCross rather than
claiming it as PE1 matching-C game logic. Exact Psy-Q routine naming is not
asserted without symbol or string provenance.

```text
POOL_DISPOSITION=SKIP-SDK-LIBRARY-COP2
MATCHING_C_COUNT=341
INTEGRATION=NONE
TIER_1_REMAINING=0
```
