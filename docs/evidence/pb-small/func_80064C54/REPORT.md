# func_80064C54 — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x80064C54`; file offset `0x55454`; size `0x2C`; unit `55454`.
- Carve: replace `[0x55454, asm]` with
  `[0x55454, c, func_80064C54]` / `[0x55480, asm]`.
- `0x55454 + 0x2C = 0x55480`; resume runs to the next span `0x55690`.

## Semantics

```
func_8005F354(func_8005DC4C(), D_8009D164);
```
`0x3F4($gp) = 0x8009D164`. Instruction-identical in shape to the already
matched neighbour `func_80064C30` (`src/func_80064C30.c`), except the first
argument is the result of `func_8005DC4C()` rather than the incoming `$a0`.

## Build profile

`era_o2_g8`.

## Matching lever

Only the gp-relative state word needs `-G8`. cc1 naturally schedules the
`addu $a0,$v0,$zero` into the second `jal` delay slot and the `lw $a1,0x3F4($gp)`
between the two calls. No source tricks required.

## Evidence

- Triage: `try_leaf.py src/func_80064C54.c 0x55454 0x2C --flags "-O2 -G8"` -> `WORDS MATCH`.
- Authority: see batch report — `EXACT SHA-1 452fb033...`, 774 leaves, `VERIFY_US=PASS`.
