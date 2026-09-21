# `func_8006FE14` — find record by +8 and tear it down

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x60614,0x60864)` = `0x250` bytes = 148 words.
- VRAM span `[0x8006FE14,0x80070064)`.
- Follows matched `func_8006FC18`; precedes matched `func_80070064`.

## Semantics (from retail bytes)

`int func_8006FE14(int x)`:

1. `x == 0` → return `-0x19`.
2. Loop **E4 arena ids 0..0xA**, then **E8 arena ids 0xB..0x15**. For any
   record whose `+8` word equals `x`:
   - `r = func_8006FC18(idx, x, 1)`; nonzero → return it.
   - On a 0 return, clear the record body; if byte 1 was `0x72`, zero
     `D_800E0EF0[0x6C..0x72]` (identical to `D_800E10A0[0..6]`) and clear
     `D_800B0CD8` bit `0x10000`.
3. Return the last `func_8006FC18` result (0 if no record matched).

## Levers

1. **`D_800E0EF0` is the real symbol for the clear loop** in this leaf
   (retail `lui $s4,%hi(D_800E0EF0)` / `addiu $s4,$s4,%lo` then
   `addiu $v1,$s4,432` = `D_800E0EF0 + 0x1B0` = `D_800E10A0`), and the
   loop index runs `0x6C..0x72` directly: `D_800E0EF0[k] = 0`. Both
   `D_800E0EF0` and `D_800E10A0` are `dlabel`s in `asm/disc1/C5060.s`, so
   the link resolves them.
2. The nested arena lookups keep the sibling form with `>= 0xB → E8`
   polarity; cc1 strength-reduces the per-loop pointers into `$s0`
   (`i*0xA0C`), `$s2` (`i*0x10C` for the E8 arm of the first loop,
   initialised to `-2948 = -0xB*0x10C`).
3. Loop over the E4 arm writes `D_800942E4 + i * 0xA0C` and the E8 arm
   `(i + 0xB) - 0xB) * 0x10C`; the second loop's E4 arm is dead but must
   still be written — retail emits it.

## Single-leaf object

```text
tools/analysis/era_leaf_match.sh src/func_8006FE14.c 0x8006FE14 0x250 -O2 -G0
```

Result: size exact 592 bytes, `MISMATCHES=29`, first at `0x8006FE38`. All
29 differing words are relocation fields.

Relocated symbols: `D_800942E4`, `D_800942E8`, `D_800E0EF0`,
`D_800B0CD8`, `func_8006FC18`.

Link-level proof:

```text
linked .text 592 bytes, target 0x250, word mismatches=0
LINK_EXACT
```

## Registration

- Source `src/func_8006FE14.c`; YAML `- [0x60614, c, func_8006FE14]`
  between `0x60864`'s neighbours.
- Default `era_o2_g0`; no profile entry needed.
