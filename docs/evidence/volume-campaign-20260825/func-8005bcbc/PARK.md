# `func_8005BCBC` — parked compiler residual

Disposition: `PARKED-GP-STATUS-REGISTER-AND-DELAY-SCHEDULE`.

## Function hood and boundaries

The retail span is `[0x8005BCBC,0x8005BD10)`, 0x54 bytes / 21 words. It
ends with `jr ra`/`nop` at `0x8005BD08/0x8005BD0C`. Exact-start callers are
at `0x8004DF28` and `0x80052834`. The preceding function returns at
`0x8005BCB4/0x8005BCB8`; the following function starts at `0x8005BD10`.
Both boundaries are real.

## Retail behavior

The function stores its input pointer in GP-backed `D_8009D0C8`. For a
non-null pointer it selects `D_800C20A4`, retaining status 9 when byte 6 is
9 and otherwise selecting status 8. For null it reads status from
`D_8009D218` and selects `D_800C0DE0`; a nonzero status advances that table
by 0x10. It stores the selected table in `D_8009D0C0` and the status in
`D_8009D0C4`.

## Bounded attempts

Both era `-O2 -G8` phrasings used the proven GP-backed globals and correct
pointer/byte semantics. The generated core was:

```text
sw a0,D_8009D0C8
beqz a0,...       [li a1,9]
lbu v0,6(a0)
lui/addiu v1,D_800C20A4
beq v0,a1,...
...
lw a1,D_8009D218
lui/addiu v1,D_800C0DE0
...
sw v1,D_8009D0C0
sw a1,D_8009D0C4
jr ra; nop
```

Retail instead initializes the result in `$v0`, materializes the non-null
table before the `lbu` (`lui a1`), and uses `$v1` for the loaded byte. The
candidate is 0x50 bytes, while retail is 0x54. Adding a return value does not
recover the retail homes; it only adds a `move v0,a1` in the return delay slot.

No pins, inline assembly, or forged padding were used. The residual is a
compiler register-home and independent-load scheduling decision, so the leaf
is not integrated and the matching-C count remains 335.
