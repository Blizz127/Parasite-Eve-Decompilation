# `func_80070064` — handler-by-byte-class sweep

Outcome: **MATCHED** on era `-O2 -G0` (default profile). Integrated as a
matching-C leaf. **`LINK_EXACT`**, 0 word mismatches at the retail VMA.

## Function hood and retail span

- File span `[0x60864,0x609B4)` = `0x150` bytes = 84 words.
- VRAM span `[0x80070064,0x800701B4)`.
- Follows matched `func_8006FE14`; precedes the PARKed `func_800701B4`.

## Semantics (from retail bytes)

`int func_80070064(void)`:

Single **E4-arena loop, ids 0..0xA** (`i * 0xA0C`). For each record whose
byte 1 is in `{0..7} ∪ {0x55..0x72}`:

- `r = func_8006FC18(i, D_8009D254, 1)`; nonzero → return it.
- On a 0 return, clear the record body; if byte 1 was `0x72`, zero
  `D_800E0EF0[0x6C..0x72]` and clear `D_800B0CD8` bit `0x10000`.
- Return 0 if nothing matched.

The byte-class test compiles to
`sltiu $v0,$v1,8` / `bnez` body / `addiu $v0,$v1,-85` /
`sltiu $v0,$v0,0x1E` / `beqz` next — i.e.
`(unsigned)h < 8 || (unsigned)(h - 0x55) < 0x1E`.

## Levers

1. **`D_8009D254` is a real `int` global** (`lw $a1,%lo(D_8009D254)`, used
   all over `asm/disc1/11718.s`), passed by value as `a1`. It is *not* an
   address.
2. `D_800E0EF0[k]` for the clear loop, index `k = 0x6C..0x72` (same
   `dlabel` identity as `func_8006FE14`).
3. The range test must be written as the two-clause OR above; a single
   `(unsigned)(h - 8) < 0x4D` fold does not reproduce retail's two
   `sltiu`s.
4. `continue` (not an `else`) keeps the loop increment block in one place.

## Single-leaf object

```text
tools/analysis/era_leaf_match.sh src/func_80070064.c 0x80070064 0x150 -O2 -G0
```

Result: size exact 336 bytes, `MISMATCHES=16`, first at `0x80070078`. All
16 differing words are relocation fields.

Relocated symbols: `D_800942E4`, `D_800942E8`, `D_800E0EF0`,
`D_800B0CD8`, `D_8009D254`, `func_8006FC18`.

Link-level proof:

```text
linked .text 336 bytes, target 0x150, word mismatches=0
LINK_EXACT
```

## Registration

- Source `src/func_80070064.c`; YAML `- [0x60864, c, func_80070064]`.
- Default `era_o2_g0`; no profile entry needed.
