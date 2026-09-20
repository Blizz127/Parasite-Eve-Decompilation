# func_80050878

- VRAM `0x80050878`, file `0x41078-0x411A8`, size `0x130` (304 B).
- Profile `era_o2_g8` (`-O2 -G8`).
- Commit `1603a5fc`, landed 884 -> 890.

Equipment-page action dispatcher. Indexes the 12-byte rows of `D_80092234` by
the gp word `D_8009CDA8` (`0x38($gp)`) times `arg0`. Case 0 rebuilds the record
for `D_8009CF04` and pushes `func_8005EB58(!enabled)`; case 2 pushes the "no
result" flag of `func_80057654`; case 3 forwards `D_8009CF08`; case 1 falls
straight through. Every path ends in
`func_80064C30(func_8005DC4C(action))`.

Iteration path:

| shape | diffs |
| --- | ---: |
| flat `int D_80092234[]` + `selected` local | 22 |
| `Row *table = D_80092234;` + `selected` local | **0** |
| `Row *row = &D_80092234[D_8009CDA8];` variant | 0 |

The row table must be a struct (`typedef struct { int v[3]; } Row;`) with an
explicit `table` pointer so cc1 computes `base + i*12` before the `arg0*4` add
and materialises `la a1,D_80092234` at function entry (retail `lui/addiu a1`
before the register saves). The `int selected = D_8009CF04;` local inside
case 0 keeps the selected index in `$s0` across `func_8005332C`, which frees
`$s1` for `action` — without it cc1 reloads `0x194($gp)` per call and swaps the
two homes (22 diffs, all downstream offsets shifted).
