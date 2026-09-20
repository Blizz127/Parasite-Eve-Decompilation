# func_80056C14 — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x80056C14`; file offset `0x47414`; size `0x2C`; unit `4680C`.
- Carve inside the `[0x4680C, asm]` run:
  `[0x4680C, asm]` / `[0x47414, c, func_80056C14]` / `[0x47440, asm]`.
- `0x47414 + 0x2C = 0x47440`; resume runs to the next span `0x48410`.

## Semantics

`return kind < 3 ? D_800A1E6E[kind << 4] : 0;` (stride `0x20` bytes, matching
`func_800314E4_port.c:54`).

## Build profile

`era_o2_g0_passthrough_load` (`-O2 -G0` + `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1`).

## Matching lever

Retail expands the indexed symbol load as the 3-word `lui $at,%hi / addu
$at,$at,$v0 / lhu $v0,%lo($at)`. The default maspsx path emits a 4-word
`lui/addiu/addu/lhu`, which both shifts the branch offset and changes the
instruction count. `MASPSX_PASSTHROUGH_SYMBOL_LOAD=1` restores the
destination-register passthrough form. The **ternary** spelling is also
required: `if (kind < 3) return ...; return 0;` makes cc1 invert the branch
(`bnez` + dead `move`) instead of retail's `beqz` with the load in the
fall-through block.

## Evidence

- Triage: `try_leaf.py src/func_80056C14.c 0x47414 0x2C --flags "-O2 -G0" --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1` -> `WORDS MATCH`.
- Authority: see batch report — `EXACT SHA-1 452fb033...`, 774 leaves, `VERIFY_US=PASS`.
