# Address-retention family screen — 2026-08-25

## Rule banked

An address-retention candidate is a retail body where a symbolic address is
materialized into a selected register and retained across the memory access,
often through the `jr ra` delay slot. The scalar-global exchange signature is:

```text
lui v1,%hi(global)
addiu v1,v1,%lo(global)
lw v0,0(v1)
jr ra
sw a0,0(v1)       # delay slot
```

The indexed-getter variant is the same compiler-pressure class with a scaled
index; `func_8007FBF0` proves that the symbolic-load gate can still leave the
retail address temporary in `$v0` while another site requires `$at`.

## Exhaustive static result

The generated retail-order asm was scanned by function body, not by symbol
name alone. Five exact scalar-exchange bodies were found:

| function | file span | global | exact-start refs | disposition |
|---|---:|---|---:|---|
| `func_800824B4` | `[0x72CB4,0x72CC8)` | `D_800B8AB0` | 0 | `SKIP-ADDRESS-RETENTION-FAMILY`; no attempt |
| `func_800824C8` | `[0x72CC8,0x72CDC)` | `D_800B8AB4` | 12 direct callers | historical `PARKED-ADDRESS-RETENTION` |
| `func_800824DC` | `[0x72CDC,0x72CF0)` | `D_800B8AB8` | screened twin | historical `PARKED-ADDRESS-RETENTION-FAMILY` |
| `func_80081E5C` | `[0x7265C,0x72670)` | `D_8009B708` | 2 direct callers | historical `PARKED-ADDRESS-RETENTION-FAMILY` |

The known indexed variant is:

| function | file span | access | exact-start refs | disposition |
|---|---:|---|---:|---|
| `func_8007FBF0` | `[0x703F0,0x70408)` | `D_8009B574[index]` | 11 direct callers | historical `PARKED-ASSEMBLER-TEMP` |

The multi-store symbolic-base variant is:

| function | file span | access | exact-start refs | disposition |
|---|---:|---|---:|---|
| `func_80082ADC` | `[0x732DC,0x73308)` | callbacks/zeros at `D_800A5AB4 + {-4,0,4,8}` | 1 direct caller | `PARKED-SYMBOLIC-BASE-RETENTION` after two phrasings |

Here retail retains one `$v0` base across all four stores. GCC 2.7.2 folds an
explicit local base back into independent symbolic stores, so maspsx expands
each through `$at`; this broadens the pressure class beyond scalar exchanges
without making every multi-store initializer an automatic skip.

`func_800824B4` has canonical `jr ra`/delay-slot semantics and real
boundaries, but no exact-start caller or relocation was found; it is retained
as a proven retail body pattern, not claimed as callable matching-C progress.

## Scheduling consequence

No new C attempt was spent. The exact family members are now represented by
the pool's address-retention skip overlay. More complex pool rows that merely
carry the broad static `address-retention` heuristic remain candidates until
their own body proves this exact family shape; the heuristic is not used to
erase unrelated loops or multi-access functions.
