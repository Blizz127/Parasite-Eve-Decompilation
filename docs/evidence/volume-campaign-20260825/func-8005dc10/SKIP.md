# `func_8005DC10` — screened address-register-coloring twin

Outcome: `SKIP-PARKED-ADDRESS-REGISTER-COLORING-FAMILY`. No C attempt or
integration; matching-C count remains 291.

Retail span `[0x4E410,0x4E428)`, six words:

```text
lui   v0,%hi(D_800A8048)
addiu v0,v0,%lo(D_800A8048)
lw    v1,0(v0)
addiu v0,v0,-0x20
jr    ra
addu  v0,v1,v0
```

It has two exact-start direct callers at `0x80055674` and `0x80057A30`, with
real boundaries on both sides and a canonical return. It is the same
address/load register-coloring family as `func_8005DBF8`: retail keeps the
symbolic address in `$v0`, loads into `$v1`, then reuses `$v0` for the offset.
The bounded 5DBF8 attempts already established that two natural C phrasings
do not close this shape; retrying the identical structure would violate the
two-iteration rule. Evidence: `func-8005dbf8/PARK.md`.
