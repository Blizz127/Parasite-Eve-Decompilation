# func_8007FBF0 (0x703F0) — PARKED-ASSEMBLER-TEMP

Status: rejected for matching-C integration; the tree remains at 281 accepted
leaves.

## Function hood

The span is `[0x703F0, 0x70408)`, six words. It ends in `jr ra` plus its
`nop` delay slot, has eleven direct `jal` callers (including `0x8007E8FC`,
`0x8007F7F8`, and `0x80082324`), and both neighboring words decode as real
instructions. It is a genuine function, not padding or a mislabeled span.

## Retail and candidate

Retail:

```text
sll a0,a0,2
lui v0,%hi(D_8009B574)
addu v0,v0,a0
lw v0,0(v0)
jr ra
nop
```

With `MASPSX_THREE_WORD_SYMBOL_STORE=1`, the candidate has the correct
three-instruction indexed-symbolic-load form, but uses `$at` as the address
temporary:

```text
sll a0,a0,2
lui at,%hi(D_8009B574)
addu at,at,a0
lw v0,0(at)
jr ra
nop
```

The alternate pointer phrasing produced the same temporary choice. The
remaining divergence is therefore in the assembler macro's register choice,
not in the C expression or cc1's source-level shape. Retail uses the load
destination `$v0` as the address temporary.

## Bounded retry result

The original candidate and both R6 retry objdumps are preserved in stash
`hold rejected 0x703F0 R6 retries`. The 3W gate fixes the length/form, but not
the `$at` versus `$v0` temporary. The gate was introduced for the `$at` form
needed by the 0x363F4-class load; this leaf shows that indexed symbolic loads
also have a destination-as-base class. This is not sufficient evidence for a
global gate change.

`matching-C = 281`; no source, YAML, build, verifier, or count integration is
claimed. The former stash `hold unaccepted 0x703F0 matching-C proposal` is
also rejected/non-matching; its disposition is recorded in the parked-blocker
entry.

Next work is a separate GNU-as/maspsx investigation of the temporary-register
discriminator, with full regression and one leaf exercising each branch before
any fourth patch.
