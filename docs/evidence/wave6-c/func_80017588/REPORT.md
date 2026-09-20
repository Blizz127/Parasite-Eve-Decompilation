# func_80017588

- VRAM `0x80017588`, file `0x7D88-0x7EB8`, size `0x130` (304 B).
- Profile `era_o2_g8` (`-O2 -G8`).
- Commit `1603a5fc`, landed 884 -> 890.

Actor action-relative cursor update. `arg0` is a two-pointer descriptor:
`*arg0[1]` is the signed relative offset and `*arg0[0]` selects the target
field. Negative offsets store 0 into `D_8009D2F0->0x1A0` / `->0x19C` or the
`D_8009D300` task at `+4`; non-negative offsets store
`D_8009D2F0->0x9C + rel*2`. Returns 1.

Codegen notes:

- `D_8009D2F0` is in gp range (gp+0x580) but retail loads it absolute
  (`lui; lw`), so it is declared as an incomplete array
  (`extern Actor *D_8009D2F0[];`). `D_8009D300` (gp+0x590) is the gp-relative
  scalar.
- Matched on the first compiled shape with the two `switch` statements kept as
  switches (the m2c draft's and the port's shapes are the same tree).

Matched on the first `try_leaf` attempt; no iteration was needed.
