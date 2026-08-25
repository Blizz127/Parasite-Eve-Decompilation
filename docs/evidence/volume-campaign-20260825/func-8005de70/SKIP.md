# `func_8005DE70` — screened address-register-coloring twin

Outcome: `SKIP-PARKED-ADDRESS-REGISTER-COLORING-FAMILY`. No C attempt or
integration; matching-C count remains 291.

Retail span `[0x4E670,0x4E688)`, six words:

```text
lui   v0,%hi(D_800A8044)
addiu v0,v0,%lo(D_800A8044)
lw    v1,0(v0)
addiu v0,v0,-0x1C
jr    ra
addu  v0,v1,v0
```

The exact-start caller is `0x80040B94`; boundaries are real and the return is
canonical. This is the same address/load register-coloring family as
`func_8005DBF8` and `func_8005DC10`, with only the symbol and offset changed.
The bounded 5DBF8 attempts already consumed the two allowed natural-C
phrasings for this shape, so this twin is screened without a duplicate
attempt.
