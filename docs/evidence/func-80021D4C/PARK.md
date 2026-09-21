# func_80021D4C — PARKED (symbol-rematerialization / base-hoist residual, -G8)

**Status:** not C-matchable with the current toolchain. Not registered in
`configs/USA/disc1.yaml`. No `src/func_80021D4C.c` is left in the tree.

## Target

- VRAM `0x80021D4C`, file `0x1254C`, span `0x94` (37 words).
- On-path fan-in **5** — the count-record list walk that clears the field
  flags after dispatching.

## Semantics (proven)

`_gp = 0x8009CD70`, so the two `$gp`-relative bytes are
`D_8009CE3C` (`gp+0xCC`, a byte count) and `D_8009D1D4` (`gp+0x464`, the
caller's cursor byte); `D_8009D1A0` is read/written **absolute** (that is why
retail reads it with `lui %hi`/`lw %lo` while the count bytes use `$gp`).

```c
i = D_8009D1D4;
while (i < D_8009CE3C) {
    short id = D_800BE834[i*8] | (D_800BE834[i*8+1] << 8);   /* 16-bit stride-8 */
    if ((unsigned)(id - 3) < 0x180)
        func_80053D2C(id - 3);
    i = (unsigned char)(i + 1);
}
D_8009D1D4 = 0;
D_8009CE3C = 0;
D_8009D1A0 &= ~0x100;
```

The `$gp`-relative count bytes force the **`-O2 -G8`** profile plus
`MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0` (the same absolute-one-global-of-a-
group pattern as `era_o2_g8_force_d8009d2f0_absolute`).

## The residual (why it cannot be closed)

Retail walks `D_800BE834` with the **symbol-relative indexed form**,
rematerializing `lui $at` / `addu $at` on every access:

```
3C01800C  lui   $at,%hi(D_800BE834)
00220821  addu  $at,$at,$v0
9424E834  lhu   $a0,%lo(D_800BE834)($at)     ; symbol + register
```

cc1 instead **hoists the base** into `$s1` (`lui $s1,%hi(D_800BE834)` /
`addiu $s1,$s1,%lo`) and walks it with `addu`/`addu`, so the loop body's
addressing mode is wrong for every iteration. `word mismatches = 34`.

This is the **same residual as `func_800374E8`** (see its PARK): a
loop-invariant symbol base that retail rematerializes per access, and cc1
hoists. Two independent on-path leaves now exhibit it.

## Levers tried (all fail)

- `-O2 -G0` (absolute form: 36 mismatches, wrong addressing entirely)
- `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0` (**34** — addressing
  right, base hoisted)
- `-O2 -G8 -fno-strength-reduce`: 34
- `-O1 -G8`: 37 (reorders the prologue further away)
- `-fno-cse -fno-strength-reduce`: link error (unreachable symbol expansion)

## Would resolve it

The same cc1/`maspsx` lever requested by `func_800374E8`: suppress
loop-invariant symbol-base hoisting, or reverse-map a hoisted base back to the
indexed symbolic form.

## Evidence

- Retail disassembly: `asm/disc1/120D8.s:305+` (`glabel func_80021D4C` ..
  `endlabel`), 37 words.
- Link check: `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0 python3
  tools/analysis/era_link_check.py src/func_80021D4C.c 0x80021D4C 0x94 -O2 -G8`
  → `word mismatches=34`.
