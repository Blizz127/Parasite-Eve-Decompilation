# `func_80079304` — screened handwritten libGTE/COP2 operation

Disposition: `SKIP-SDK-LIBRARY-COP2`. No ordinary-C phrasing, carve,
integration, stash, or matching-C count change was made. The matching-C count
remains 338.

## Function hood and exact boundaries

Retail span `[0x69B04,0x69B7C)`, VRAM `[0x80079304,0x8007937C)`, is
`0x78` bytes / 30 words. It ends in canonical `jr ra` at `0x80079374`, with
the live returned-value shift `sra v0,v0,2` in its delay slot at
`0x80079378`.

Two distinct instructions call the exact start, both encoding `0C01E4C1`:

```text
caller PC   file off
800C53BC    0B5BBC
800C5C60    0B6460
```

The functional boundaries are explicit, but the immediate adjacent words are
alignment rather than instructions. The preceding real handwritten
`func_800792D4` returns with a live store at `0x69AF4/0x69AF8`; two alignment
nops at `0x69AFC/0x69B00` lead to this exact-start called body. Two alignment
nops at `0x69B7C/0x69B80` follow it, and real handwritten
`func_80079384` starts at `0x69B84` with `lwc2 $0,0(a0)`. The symmetric
padding lies outside the called span.

```text
FUNCTION_HOOD=PROVEN_BY_2_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine callable function, not padding, data, or a tail fragment.
The old pool shorthand `boundaries=real/real` is refined here to the actual
real-function / two-nop-padding / function geometry.

## Full retail body

The generated split explicitly marks the function handwritten:

```text
file    word      instruction
69B04   C8800000  lwc2  $0,0(a0)
69B08   C8810004  lwc2  $1,4(a0)
69B0C   C8A20000  lwc2  $2,0(a1)
69B10   C8A30004  lwc2  $3,4(a1)
69B14   C8C40000  lwc2  $4,0(a2)
69B18   C8C50004  lwc2  $5,4(a2)
69B1C   00000000  nop
69B20   4A280030  rtpt
69B24   8FA80010  lw    t0,0x10(sp)
69B28   8FA90014  lw    t1,0x14(sp)
69B2C   8FAA0018  lw    t2,0x18(sp)
69B30   E90C0000  swc2  $12,0(t0)
69B34   E92D0000  swc2  $13,0(t1)
69B38   E94E0000  swc2  $14,0(t2)
69B3C   4843F800  cfc2  v1,$31
69B40   C8E00000  lwc2  $0,0(a3)
69B44   C8E10004  lwc2  $1,4(a3)
69B48   00000000  nop
69B4C   4A180001  rtps
69B50   8FA8001C  lw    t0,0x1C(sp)
69B54   8FA90020  lw    t1,0x20(sp)
69B58   8FAA0024  lw    t2,0x24(sp)
69B5C   E90E0000  swc2  $14,0(t0)
69B60   E9280000  swc2  $8,0(t1)
69B64   4848F800  cfc2  t0,$31
69B68   48029800  mfc2  v0,$19
69B6C   01034025  or    t0,t0,v1
69B70   AD480000  sw    t0,0(t2)
69B74   03E00008  jr    ra
69B78   00021083  sra   v0,v0,2
```

The first six transfers load three caller vectors into COP2 data registers;
`rtpt` performs the three-point operation and the three `swc2` instructions
store its results. A fourth vector is loaded for `rtps`; two more COP2 values
are stored, the two operation flag values are combined, and data register 19
is shifted for the return value. This instruction-level contract is proven.
No exact Psy-Q routine name is assigned without symbol or string provenance.

## Screens and disposition

| screen | result |
|---|---|
| callee buckets | no `jal`; the two GTE operations are architectural instructions, not C callees |
| written-global Stage 0 | no symbolic global; only caller-supplied result pointers from stack arguments are written |
| coloring pressure | six COP2 inputs, five stack-supplied outputs, two flag values, and one returned COP2 value; irrelevant to ordinary C once the architectural effects are required |
| `$v0` liveness | COP2 data register 19 is copied to `v0` and shifted in the return delay slot |
| address retention | argument and stack-supplied pointers only; no symbolic address |
| optimization signal | none: explicit handwritten marker and literal COP2 operations |
| loop/back-edge owner | none |
| ordinary-C expressibility | required `lwc2`, `rtpt`, `rtps`, `swc2`, `cfc2`, and `mfc2` effects have no sanctioned intrinsic in this toolchain |

Omitting those operations would remove the function's semantics. Pins cannot
create COP2 operations, and inline or file-scope assembly is forbidden as
matching-C progress. This is the same proven libGTE/PsyCross redirect class as
the adjacent handwritten COP2 operation helpers, not a cc1 residual and not a
matching leaf.

```text
POOL_DISPOSITION=SKIP-SDK-LIBRARY-COP2
C_PHRASINGS=0
INTEGRATION=NONE
MATCHING_C_COUNT=338
```
