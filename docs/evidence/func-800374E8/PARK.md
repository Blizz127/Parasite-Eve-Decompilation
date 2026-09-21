# func_800374E8 — PARKED (symbol-rematerialization / base-hoist residual)

**Status:** not C-matchable with the current toolchain. Not registered in
`configs/USA/disc1.yaml`. No `src/func_800374E8.c` is left in the tree.

## Target

- VRAM `0x800374E8`, file `0x27CE8`, span `0x60` (24 words).
- On-path fan-in **7** — a 4-slot descriptor clear called from the boot
  `0x80037xxx` cluster.

## Semantics (proven)

Two parallel arrays with a **56-byte (0x38) stride**. For `i = 0..3`:

```c
D_800BCEA8[i * 56] = 0;
D_800BCEB4[i * 56] &= 0xFDFFFFFF;   /* clear bit 25 */
```

The stride is fixed by retail's own index computation:

```
andi  $v1,$a0,0xFF      ; i & 0xFF
sll   $v0,$v1,3
subu  $v0,$v0,$v1       ; i*8 - i = i*7
sll   $v0,$v0,3         ; i*56
```

The loop bound is the *masked* counter (`andi $v0,$a0,0xFF; sltiu $v0,4`), and
the mask constant `0xFDFFFFFF` lives in `$a1` (`lui 0xFDFF` / `ori 0xFFFF`).

## The residual (why it cannot be closed)

Retail addresses **both** symbols with the symbol-relative indexed form, using
`$at` as the address temp and **rematerializing `lui $at` on every access** —
three separate `lui $at` / `addu $at` pairs in the loop body:

```
lui   $at,%hi(D_800BCEB4)          ; 27D04
addu  $at,$at,$v0
lw    $v1,%lo(D_800BCEB4)($at)     ; symbol + register
...
lui   $at,%hi(D_800BCEA8)          ; 27D14
addu  $at,$at,$v0
sb    $zero,%lo(D_800BCEA8)($at)   ; symbol + register, in the delay slot
...
lui   $at,%hi(D_800BCEB4)          ; 27D24
addu  $at,$at,$v0
sw    $v1,%lo(D_800BCEB4)($at)     ; symbol + register
```

cc1 instead **hoists the symbol base into a callee-saved register** (the base is
loop-invariant) and walks it with `addiu`, so the linked words are
`lui $3,%hi(SYM)` / `addiu $3,$3,%lo` / `lw $2,0($3)` rather than the
`$at`-indexed form. `word mismatches = 20` at `-O2 -G0`.

This is the inverse of the `func_8002F9CC` lever: there, declaring the real
aggregate element *prevented* cc1 from hoisting an address and let
`MASPSX_THREE_WORD_SYMBOL_STORE` pick retail's three-word symbol store. Here cc1
**does** hoist, and `maspsx` cannot rewrite a hoisted register base back into an
indexed symbolic access.

## Levers tried (all fail)

- **Flat arrays vs. a 56-byte aggregate element (with the word at +0xC):** both
  hoist the base. (The struct variant additionally walks the base with
  `addiu $4,$4,0x38`, moving further from retail.)
- **`volatile`** on either/both arrays: no change (20).
- **Local per-slot pointers** inside the loop (`unsigned char *p = &...`,
  `unsigned int *q = &...`): 20.
- **Masked `unsigned char` loop counter** (`(unsigned char)i < 4`): 20.
- **Flag rungs:** `-O2 -G0`, `-O1 -G0`, `-O1 -G0 -fschedule-insns2`,
  `-O2 -G0 -fno-strength-reduce` — none removes the hoist; `-O1` reorders the
  prologue further away.

## Would resolve it

A cc1/`maspsx` lever that suppresses loop-invariant symbol-base hoisting (or
that reverse-maps a hoisted base back to the symbol-relative indexed form) — the
same class as the `func_800701B4` / `func_80073A44` scheduling parks. None
exists today.

## Evidence

- Retail disassembly: `asm/disc1/27C6C.s` (`glabel func_800374E8` .. `endlabel`),
  24 words.
- Link check: `python3 tools/analysis/era_link_check.py src/func_800374E8.c
  0x800374E8 0x60 -O2 -G0` → `word mismatches=20`.
