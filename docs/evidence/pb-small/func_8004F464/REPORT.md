# func_8004F464 — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x8004F464`; file offset `0x3FC64`; size `0x2C`; unit `3FC64`.
- Carve: replace `[0x3FC64, asm]` with
  `[0x3FC64, c, func_8004F464]` / `[0x3FC90, asm]`.
- `0x3FC64 + 0x2C = 0x3FC90`; resume runs to the next span `0x40008`.

## Semantics

```
if (D_8009D008 != 0) { func_8004E97C(); D_8009D008 = 0; }
```
`0x298($gp) = 0x8009D008`. Matches `func_8005C498_port.c:66` (the port treats
`func_8004E97C` as an unresolved menu boundary; the retail C is a plain call).

## Build profile

`era_o2_g8`.

## Matching lever

The load and the clear are both gp-relative on the same 4-byte scalar, so
`-G8` is sufficient; the `beqz` on the loaded value naturally fills the delay
slot with the `$ra` store. No source tricks required.

## Evidence

- Triage: `try_leaf.py src/func_8004F464.c 0x3FC64 0x2C --flags "-O2 -G8"` -> `WORDS MATCH`.
- Authority: see batch report — `EXACT SHA-1 452fb033...`, 774 leaves, `VERIFY_US=PASS`.
