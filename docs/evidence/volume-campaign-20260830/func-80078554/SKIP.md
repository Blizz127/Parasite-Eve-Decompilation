# `func_80078554` — screened handwritten libGTE/COP2 interpolation helper

Disposition: `SKIP-SDK-LIBRARY-COP2`. No ordinary-C phrasing, carve,
integration, stash, or matching-C count change was made. The matching-C count
remains 339.

## Function hood and exact boundaries

Retail span `[0x68D54,0x68DD0)`, VRAM `[0x80078554,0x800785D0)`, is
`0x7C` bytes / 31 words. It ends in canonical `jr ra; nop` at
`0x800785C8/0x800785CC`.

Two distinct instructions call the exact start. Both encode `0C01E155`:

```text
caller PC   file off   call
800CF49C    0BFC9C     jal 80078554
800D34A4    0C3CA4     jal 80078554
```

The preceding real handwritten `func_800784F4` ends immediately before this
span with `jr ra` at `0x68D4C` and its real delay-slot `nop` at `0x68D50`.
After this function's own return delay slot, the word at `0x68DD0` is one
alignment `nop`; real handwritten `func_800785D4` begins at `0x68DD4` with
`lw t0,0(a0)`. Thus the called span is bounded by the preceding function and
by a one-word inter-function gap before the following function. The pool's
old `real/real` shorthand is refined here to the exact geometry.

```text
FUNCTION_HOOD=PROVEN_BY_2_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine callable function, not padding, data, or a tail fragment.

## Full retail body

The generated split explicitly marks the function handwritten:

```text
file    word      instruction
68D54   90880000  lbu   t0,0(a0)
68D58   90890001  lbu   t1,1(a0)
68D5C   908A0002  lbu   t2,2(a0)
68D60   48864000  mtc2  a2,$8
68D64   48884800  mtc2  t0,$9
68D68   48895000  mtc2  t1,$10
68D6C   488A5800  mtc2  t2,$11
68D70   00000000  nop
68D74   4B90003D  gpf   0
68D78   90A80000  lbu   t0,0(a1)
68D7C   90A90001  lbu   t1,1(a1)
68D80   90AA0002  lbu   t2,2(a1)
68D84   4802F800  mfc2  v0,$31
68D88   48874000  mtc2  a3,$8
68D8C   48884800  mtc2  t0,$9
68D90   48895000  mtc2  t1,$10
68D94   488A5800  mtc2  t2,$11
68D98   240B000C  addiu t3,zero,0xC
68D9C   4BA0003E  gpl   0
68DA0   8FAD0010  lw    t5,0x10(sp)
68DA4   4808C800  mfc2  t0,$25
68DA8   4809D000  mfc2  t1,$26
68DAC   480AD800  mfc2  t2,$27
68DB0   01684007  srav  t0,t0,t3
68DB4   01694807  srav  t1,t1,t3
68DB8   016A5007  srav  t2,t2,t3
68DBC   A1A80000  sb    t0,0(t5)
68DC0   A1A90001  sb    t1,1(t5)
68DC4   A1AA0002  sb    t2,2(t5)
68DC8   03E00008  jr    ra
68DCC   00000000  nop
```

At instruction level, the helper loads three bytes from each of two caller
buffers, applies the caller's two fixed-point coefficients through GTE
`GPF`/`GPL`, shifts the three MAC results by 12, and stores three result bytes
through the fifth argument. It also leaves the first GTE flag read in `v0`.
This contract is proven from the body; no exact Psy-Q routine name is assigned
without symbol or string provenance.

## Screens and disposition

| screen | result |
|---|---|
| callee buckets | no `jal`; `gpf` and `gpl` are architectural COP2 operations, not C callees |
| written-global Stage 0 | no symbolic global is read or written; only the fifth caller-supplied pointer receives three bytes |
| coloring pressure | two three-byte inputs, two coefficients, a stack-supplied output, GTE MAC registers, and the GTE flag result; irrelevant once architectural effects are required |
| `$v0` liveness | `mfc2 v0,$31` at `0x68D84` survives to the canonical return |
| address retention | argument and stack-supplied pointers only; no symbolic address |
| optimization signal | none: explicit handwritten marker and literal COP2 operations |
| loop/back-edge owner | none |
| ordinary-C expressibility | required `mtc2`, `gpf`, `gpl`, and `mfc2` effects have no sanctioned intrinsic in this toolchain |

Replacing the GTE operations with scalar arithmetic would not reproduce the
architectural side effects or instruction stream. Pins cannot create COP2
operations, and inline or file-scope assembly is forbidden as matching-C
progress. This is the established handwritten libGTE/PsyCross redirect class,
not a cc1 residual and not a matching leaf.

```text
POOL_DISPOSITION=SKIP-SDK-LIBRARY-COP2
C_PHRASINGS=0
INTEGRATION=NONE
MATCHING_C_COUNT=339
```
