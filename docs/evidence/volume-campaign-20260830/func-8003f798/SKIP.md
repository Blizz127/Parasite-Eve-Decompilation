# `func_8003F798` — screened handwritten COP2 helper

No C phrasing was attempted and no matching leaf is claimed. The matching-C
count remains 337.

## Function hood and boundaries

Retail span `[0x2FF98,0x30000)`, VRAM `[0x8003F798,0x8003F800)`, is
`0x68` bytes / 26 words. It ends in canonical `jr ra; nop` at
`0x8003F7F8/0x8003F7FC`.

Three distinct instructions call the exact start:

```text
caller PC   file off  encoded jal
8003F11C    02F91C    0C00FDE6
8003F16C    02F96C    0C00FDE6
8003F1D0    02F9D0    0C00FDE6
```

The preceding real handwritten function `func_8003F758` ends with `jr ra`
and a live `sh zero,0x10(a0)` delay slot at file `0x2FF90/0x2FF94`. The
following real function `func_8003F800` begins immediately at `0x30000` with
`lui a3,%hi(D_800A0ED0)`. Both boundary sides are executable instructions.

```text
FUNCTION_HOOD=PROVEN_BY_3_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine callable function, not padding, data, or a tail fragment.

## Full retail body

The generated split explicitly marks the function handwritten. Its complete
26-word body is:

```text
file    word      instruction
2FF98   97A30010  lhu    v1,0x10(sp)
2FF9C   2CA20003  sltiu  v0,a1,3
2FFA0   10400005  beqz   v0,0x8003F7B8
2FFA4   00051040  sll    v0,a1,1
2FFA8   00441021  addu   v0,v0,a0
2FFAC   A4460020  sh     a2,0x20(v0)
2FFB0   A4470026  sh     a3,0x26(v0)
2FFB4   A443002C  sh     v1,0x2C(v0)
2FFB8   00051080  sll    v0,a1,2
2FFBC   00821021  addu   v0,a0,v0
2FFC0   A0460040  sb     a2,0x40(v0)
2FFC4   A0470041  sb     a3,0x41(v0)
2FFC8   A0430042  sb     v1,0x42(v0)
2FFCC   24820020  addiu  v0,a0,0x20
2FFD0   8C4C0000  lw     t4,0(v0)
2FFD4   8C4D0004  lw     t5,4(v0)
2FFD8   48CC8000  ctc2   t4,$16
2FFDC   48CD8800  ctc2   t5,$17
2FFE0   8C4C0008  lw     t4,8(v0)
2FFE4   8C4D000C  lw     t5,12(v0)
2FFE8   8C4E0010  lw     t6,16(v0)
2FFEC   48CC9000  ctc2   t4,$18
2FFF0   48CD9800  ctc2   t5,$19
2FFF4   48CEA000  ctc2   t6,$20
2FFF8   03E00008  jr     ra
2FFFC   00000000  nop
```

The ordinary memory effects update three halfword lanes for indices below
three, update a three-byte indexed record, then load five words from the
argument block at `+0x20..+0x30`. The five final architectural effects are
writes to GTE control registers 16 through 20. This instruction-level role is
proven without assigning an unverified Psy-Q routine name.

## Screens and disposition

| screen | result |
|---|---|
| callee buckets | no `jal`; no callees |
| Stage-0 writes | argument-relative stores only; no symbolic global |
| coloring / `$v0` / address retention | irrelevant once the architectural COP2 effects are required |
| optimization signal | source split says handwritten; five literal `ctc2` operations |
| loop / back-edge | none |
| ordinary-C expressibility | five required GTE control-register writes have no sanctioned intrinsic in this toolchain |

Omitting the five `ctc2` operations would omit required side effects. The
repository-wide COP2 screen has already established that pins, inline
assembly, and file-scope assembly are forbidden and that no sanctioned
ordinary-C intrinsic exists. A C attempt would therefore only rediscover the
known instruction-set expressibility blocker.

This mixed memory/COP2 helper is kept in the conservative handwritten class;
the evidence does not prove that it belongs to the separately enumerated
contiguous libGTE SDK family at `func_80078E04..func_80079024`.

```text
POOL_DISPOSITION=SKIP-HANDWRITTEN-COP2
C_PHRASINGS=0
INTEGRATION=NONE
MATCHING_C_COUNT=337
```
