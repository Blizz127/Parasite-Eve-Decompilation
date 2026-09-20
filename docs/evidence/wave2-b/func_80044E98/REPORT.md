# func_80044E98 — wave2-b

- Leaf: `func_80044E98` (VRAM 0x80044E98)
- File: 0x35698, size 0xF4 (61 words), unit 35698
- Source: `src/func_80044E98.c`
- Build profile: `era_o2_g8` (`-O2 -G8`), no maspsx gates
- YAML: head of the `[0x35698, asm]` run, carved to
  `[0x35698, c, func_80044E98]`, `[0x3578C, asm]`

## Result

Fresh `scripts/build_us.sh` prints EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; registered C leaves 810 → 815
with the sibling leaves landed in the same wave. `scripts/verify_us.sh`
= PASS at 815.

## Divergences / levers

1. **Switch, not if/else.** Retail's compare chain is
   `beqz $v1, case0` / `addiu $v0,1` / `beq $v1,$v0,case1` / `j default`.
   Writing the dispatch as `switch (func_80063428(...))` with `case 0`,
   `case 1: goto case1;`, `default: return 1;` reproduces that layout.
   An `if (v0 == 0) {...} if (v0 != 1) return 1;` chain lays the case-0
   block out first and shifts every later branch.
2. **Call arguments.** Retail passes `(arg0, 0)` to `func_80062A20`
   (`$a0` untouched, `$a1 = 0`); m2c's draft printed `func_80062A20(0)`.
   `func_80062A20` is `(unsigned int *a0, unsigned int a1)` in
   `src/func_80062A20.c`.
3. **gp slots.** `0x238($gp)` = `D_8009CFA0 + 8`, declared as a scalar and
   loaded as a `Fn` through `*(Fn *)((char *)&D_8009CFA0 + 8)` so the
   callback test/`jalr` shape matches.
