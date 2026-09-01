# `func_800534CC` — parked gp-small-data form

Outcome: `PARKED-GP-ABSOLUTE-FORM` after two bounded natural-C phrasings. No
integration; matching-C count remains 291.

## Function hood and retail body

Retail span `[0x43CCC,0x43CE4)`, six words. It ends in `jr ra` plus delay-slot
`nop`; the preceding word at `0x43CC8` is the prior function's delay slot and
the following real function begins at `0x43CE4`. One exact-start caller is
`0x80050414`; `FUNCTION_HOOD=PROVEN`.

```text
800534CC: lw   v0,0x2D8(gp)
800534D0: sll  a0,a0,1
800534D4: addu a0,a0,v0
800534D8: lh   v0,0(a0)
800534DC: jr   ra
800534E0: nop
```

The helper is a gp-indexed signed-halfword getter over the small-data object
`D_8009D2D8`. It has no callee, frame, loop, or written global. The key screen
is the required `$gp+0x2D8` base load; no pins or inline assembly are allowed.

## Bounded attempts

Both attempts used `era_compile ... -O2 -G8`.

Attempt 1:

```c
extern short D_8009D2D8[];
short func_800534CC(int a0) { return D_8009D2D8[a0]; }
```

Attempt 2:

```c
extern short D_8009D2D8;
short func_800534CC(int a0) { return (&D_8009D2D8)[a0]; }
```

Both produced the same absolute-address form at the target rather than the
retail gp-relative load. The full-build result was:

```text
RESULT: NON-MATCH (22/2025472 bytes differ, 0.0011%)
first mismatch @ file 0x43CCC: cand=0x40 orig=0xD8
```

The object body was six words after zero-pad trim, so this is a code-generation
form mismatch, not a carve or padding error. R6 parks after two iterations.
Candidate source and integration changes are preserved in stash `park volume
func_800534CC gp-small-data form`; no YAML/build integration remains in the
tree.

## Carve geometry recorded for the rejected integration

Prior asm span `[0x43724,0x44A88)`, size `0x1364`:

```text
prefix:  0x43CCC - 0x43724 = 0x5A8
C leaf:  0x43CE4 - 0x43CCC = 0x18
resume:  0x44A88 - 0x43CE4 = 0xDA4
closure: 0x5A8 + 0x18 + 0xDA4 = 0x1364
```
