# func_80026FD0 — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x80026FD0`; file offset `0x177D0`; size `0x28`; unit `120D8`.
- Carve inside the `[0x120D8, asm]` run:
  `[0x120D8, asm]` / `[0x177D0, c, func_80026FD0]` / `[0x177F8, asm]`.
- `0x177D0 + 0x28 = 0x177F8`; the asm resume runs to the next span `0x19B88`.

## Semantics

```
if (D_8009D2B0 != 0) { D_8009CE68 = 0x80; D_8009CE6C = -8; }
```
`0xF8($gp) = 0x8009CE68`, `0xFC($gp) = 0x8009CE6C` (gp base `0x8009CD70`),
matching `func_80027D14_port.c:345`.

## Build profile

`era_o2_g8`.

## Matching lever

- Both destinations are gp-relative byte stores -> `-G8`.
- `D_8009D2B0` is loaded **absolute** (`lui`/`lb`); declaring it as an
  incomplete array (`extern signed char D_8009D2B0[];`) keeps it out of small
  data.
- **Constant typing matters**: `D_8009CE68` must be `unsigned char` (so `0x80`
  materializes as `addiu $v0,$zero,0x80`, not the sign-extended `0xFF80`), while
  `D_8009CE6C` is `signed char` (so `-8` materializes as `addiu $v0,$zero,-8`,
  not `addiu $v0,$zero,0xF8`). This was the only divergent word in the first
  attempt (`0x0010: retail 24020080 cand 2402FF80`).

## Evidence

- Triage: `try_leaf.py src/func_80026FD0.c 0x177D0 0x28 --flags "-O2 -G8"` -> `WORDS MATCH`.
- Authority: see batch report — `EXACT SHA-1 452fb033...`, 774 leaves, `VERIFY_US=PASS`.
