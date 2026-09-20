# func_8005FA3C

- VRAM `0x8005FA3C`, file `0x5023C-0x50374`, size `0x138` (312 B).
- Profile `era_o2_g8_expand_div` (`-O2 -G8` + `MASPSX_EXPAND_DIV=1`, new).
- Commit `1603a5fc`, landed 884 -> 890.

Two-digit signed decimal emitter. Negative input is negated with the `0x52`
minus glyph and the field shrinks to one digit; `divisor = 10^(digits-1)`;
each iteration calls `func_8005F874` with `-1` for a suppressed leading zero
and advances the gp text cursor pair `D_8009D124 += 5` /
`*p = D_8009D128` (`p = &D_8009D128`), then `divisor /= 10`.

Match levers:

- `-O2 -G8` is required (all cursor accesses are gp-relative). The default
  maspsx leaves the signed division unchecked; retail has the explicit
  `bnez s1; break 7; ...; bne s3,at; break 6; mflo` sequence, so the leaf needs
  `MASPSX_EXPAND_DIV=1`. Candidate goes 288 -> 312 bytes with the gate.
- The suppressed-zero argument must be the **ternary**
  `func_8005F874(i < digits - 1 && digit == 0 ? -1 : digit)`. Only this form
  lets the scheduler hoist `slt v0,s0,s5` ahead of the `div` and place the
  next-iteration copy in the loop-back delay slot; an explicit `lt` local, an
  m2c-style do/while, and a split `if (lt && digit == 0)` all leave the `slt`
  after `mflo` (14 diffs).
- The `int *p = &D_8009D128; D_8009D124 += 5; *p = D_8009D128;` idiom keeps
  the redundant `+0x3B8($gp)` self-store (same idiom as `func_800605F8`).
