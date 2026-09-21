# `func_800702DC` — E4-arena teardown sweep

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x60ADC,0x60BF4)` = `0x118` bytes = 70 words.
- VRAM span `[0x800702DC,0x800703F4)`.
- Last leaf of the `0x601F0..0x60BF4` asm block; precedes matched
  `func_800703F4`.

## Semantics (from retail bytes)

`int func_800702DC(void)`:

Single **E4-arena loop, ids 0..0xA**: `r = func_8006FC18(i, 0, 1)`; nonzero
→ return it; on a 0 return clear the record body, and if byte 1 was `0x72`
zero `D_800E0EF0[0x6C..0x72]` and clear `D_800B0CD8` bit `0x10000`. Return
0 if nothing matched. Same shape as `func_80070064` without the byte-class
filter and with `a1 = 0`.

## Levers

1. **`i` is the loop counter *and* the call argument** — no separate `idx`
   local. cc1 allocates `$s0 = i`, strength-reduces `i * 0xA0C` to `$s2`
   (init 0) and `(i - 0xB) * 0x10C` to `$s1` (init `-2948`), matching
   retail exactly. Introducing an `idx` biv (as `func_800701B4` needs)
   perturbs the callee-saved allocation order and does not link-exact.
2. `D_800E0EF0[k]` clear loop, index `k = 0x6C..0x72`.
3. Same nested-arena polarity (`>= 0xB → E8`) as the siblings.
4. `r = 0;` before the loop is dead (the loop always runs once) and is
   eliminated by cc1; the first `$a2` write is `move $a2,$v0` from the
   `func_8006FC18` return, as retail.

## Single-leaf object

```text
tools/analysis/era_leaf_match.sh src/func_800702DC.c 0x800702DC 0x118 -O2 -G0
```

Result: `SIZE_MISMATCH C=0x120 ROM=0x118` (GNU as pad), `MISMATCHES=12`,
first at `0x800702EC`. All 12 differing words are relocation fields.

Relocated symbols: `D_800942E4`, `D_800942E8`, `D_800E0EF0`,
`D_800B0CD8`, `func_8006FC18`.

Link-level proof:

```text
linked .text 288 bytes, target 0x118, word mismatches=0
LINK_EXACT
```

## Registration

- Source `src/func_800702DC.c`; YAML `- [0x60ADC, c, func_800702DC]`
  between the PARKed `func_800701B4` asm span (`0x609B4`) and
  `func_800703F4` (`0x60BF4`).
- Default `era_o2_g0`; no profile entry needed.
