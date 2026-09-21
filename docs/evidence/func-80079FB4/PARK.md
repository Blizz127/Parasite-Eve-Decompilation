# func_80079FB4 — PARKED

VRAM `0x80079FB4`, size `0x174` (93 words), `asm/disc1/68530.s`.
**Top on-path fan-in remaining (19)** in `tools/analysis/route_coverage.py`.

Semantics are clear: a signed arctan-style ratio returning `[0,0x800]` from the
`D_8009A6EC` short table, with two sign flags and two div-by-`>>10` guarded
quotients. The control flow is legible and the C below is *semantically*
correct, but it does not reproduce retail's block layout.

## Residual

Retail's zero-guard shape is:

```
bnez $a1,.L80079FEC
 slt  $v0,$a0,$a1        ; delay slot
beqz $a0,.L8007A120
 addu $v0,$zero,$zero
slt   $v0,$a0,$a1        ; <-- duplicated
.L80079FEC:
```

i.e. the comparison is materialized **twice** (once in the first branch's delay
slot, once again after the zero test) before the `beqz` on the shared compare.
A plain `if (a1 == 0) { if (a0 == 0) return 0; }` produces a single `slt`, and
the `&&` form (`if (a1 == 0 && a0 == 0)`) produces a different arm entirely, so
the duplicate is a source-level block-layout artifact that no flag rung or
operand reorder reproduced.

Additionally retail expands both `div` sites with full ASPSX guard ladders
(`break 7` / `-1` / `0x80000000` / `break 6`) inline in the middle of the two
ratio arms — 2 words longer than the plain `div` cc1 emits even with
`MASPSX_EXPAND_DIV=1`. No combination of `MASPSX_EXPAND_DIV` and
`MASPSX_SYMBOL_AT_TEMP` closed it.

## What was tried

```
signature: int f(int a0, int a1)
rungs:     -O2 -G0, -O2 -G0 -fschedule-insns2, -O1 -G0
knobs:     MASPSX_EXPAND_DIV=1, MASPSX_SYMBOL_AT_TEMP=1,
           MASPSX_SYMBOL_LOAD_DEST_TEMP=1, combinations
variants:  nested zero test, `&&` zero test, na/nb swap, table-index via
           int/unsigned locals
best:      MASPSX_EXPAND_DIV=1 + patch 5 -> 51 object mismatches
```

Do not retry without a genuinely new structural lever (e.g. a fresh cc1
block-merge knob that re-materializes a shared compare, or a maspsx patch class
that duplicates compare materialization into a branch delay slot).
