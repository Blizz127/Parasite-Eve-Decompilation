# func_8006A25C — landed (771 → 772)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x8006A25C · **file:** 0x5AA5C · **size:** 0x8C (35 words)
**Unit:** 56438.s (mid-carve), prefix 0x56438..0x5AA5C, resume 0x5AAE8..0x5ADBC
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_8006A25C.c 0x5AA5C 0x8C --flags "-O2 -G0"` | `WORDS MATCH (+4 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (772 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

Sound/CD teardown sequence `func_80081268()`, `func_80086F34(0)`,
`func_80086FF8()`, `func_80087024()`, `func_80085744()`; if
`func_80038D0C() & 0xFF` is nonzero, `func_80039970()`. Then it snapshots
`D_8009D280` into `D_800A77F4`, overwrites `D_8009D280` with the
game-over destination `0xA9400048`, and sets bit 0x100 in `D_800B0CD8`.

## Levers

1. **Void return + reordered tail.** The matching source is
   ```
   p = &D_800B0CD8;
   t = D_8009D280;
   D_800A77F4 = t;
   D_8009D280 = 0xA9400048;
   *p = *p | 0x100;
   ```
   with `void func_8006A25C`. The "natural" `t = ...; D_8009D280 = ...;
   *p |= ...; D_800A77F4 = t;` order (and/or an `int` return) makes cc1
   allocate the 0xA9400048 constant to `$v1` and the saved destination to
   `$a1` — retail is the opposite — a stable 3-word diff.
2. **`unsigned int *p = &D_800B0CD8` local** gives the shared `$a0` base
   (`lui $a0,%hi; addiu $a0,%lo; lw $v0,0($a0) ... sw $v0,0($a0)`) instead
   of two independent `lui $at` absolute accesses.
3. Declaring the pointer **before** `t` matters for register order.

## Divergences / negatives

* `int`-returning variants: 3-word diff (constant in `$v1` not `$a1`).
* `register ... asm("$3")` pins on the saved destination: worse (4-5 diffs).
* The `lui $a1,0xA940` appears twice in retail (once speculatively in the
  `beqz` delay slot, once after the conditional call) — reproduced by the
  matching form.
