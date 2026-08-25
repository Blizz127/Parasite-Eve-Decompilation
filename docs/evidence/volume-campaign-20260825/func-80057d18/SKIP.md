# `func_80057D18` — screened gp-indexed getter/clear twin

Outcome: `SKIP-PARKED-GP-ABSOLUTE-FORM-FAMILY`. No C attempt or integration;
matching-C count remains 291.

Retail span `[0x48518,0x48530)`, six words:

```text
lw   v0,0x2D8(gp)
sll  a0,a0,1
addu a0,a0,v0
lh   v0,0(a0)
jr   ra
sh   zero,0(a0)
```

Function hood is proven by the canonical return, real boundaries, and the
exact-start function-pointer construction at file `0x34D00`:
`lui v1,%hi(func_80057D18)` / `addiu v1,v1,%lo(func_80057D18)`.

This is the same gp-indexed `D_8009D2D8` family as `func_800534CC`, with a
post-read clear store. The 534CC two-phrasing attempt established that the
current natural gp forms emit an absolute address rather than retail's
`$gp+0x2D8` load. Repeating the same compiler-form attempt would violate the
two-iteration rule, so this twin is screened without consuming an attempt.
