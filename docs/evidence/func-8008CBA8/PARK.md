# PARK: func_8008CBA8 (0x8008CBA8, 0x3C8 = 242 words)

**Status: PARKED — near-miss, 204/242 word mismatches, compiled .text 0x3A0
(232 words) vs retail 0x3C8 (242 words).**

`func_8008CBA8` is the `D_800BCD80` command dispatcher whose 40-setter family is
already registered (`docs/evidence/D_800BCD80_SETTERS/REPORT.md`). It is a large
`switch (D_800BCD80)` over `0x10/0x12/0x19`, `0x24`, `0x98`, `0x99`, `0xD8`,
`0xD9`, `0xDA` and a default arm.

## What is proven correct (matches retail exactly)

* Dispatch order discovered by iterating: the three `D_800BCD80` values that
  share the first body are tested **before** the `0x24` arm, and the case bodies
  are emitted in the order `0xD8`, `0xD9`, `0xDA`, `0x99`, `0x98`, then default.
  With that order the entire comparison tree, the `sltiu` chain, the constant
  materialisation order and the final `j` targets all match retail exactly
  (zero diffs in the first 33 instructions once the callee-saved set is right).
* The `0x24` arm is byte-exact in isolation: `func_8008CB08(&rec)`, store
  `D_800BCD84`/`88`/`8C`/`90` into `rec[1..4]`, `ret = D_8009CDF0`,
  `D_8009CDF0 = ((ret + 1) & 0x1FF) + 0x400`, `rec[5] = ret`, `rec[0] = cmd`.
* The `0x98`/`0x99`/`0xD8`/`0xD9`/`0xDA`/default bodies are structurally right
  (two `func_8008CB08` calls, constant writes to `rec[0]`, selected
  `D_800BCD8x` globals into `rec[1..4]`).
* `cmd` must be read **once** into a local and the switch performed on it:
  `switch (D_800BCD80)` directly, or reloading the global inside an arm, makes
  cc1 emit an extra `lui`/`lw` pair and the dispatch tree diverges.
* The function takes **no arguments** and returns `int` (`$v0 = $s1`).

## Residual (why it is parked)

Three intertwined cc1 allocation/scheduling differences:

1. **Callee-saved set.** Retail saves `$s0` (=`&D_800BCD80`), `$s1` (=the return
   value accumulator, initialised `0` at the very top and returned as `$v0`) and
   `$s2` (=`D_800BCD84`, the first-body cursor). cc1 with a natural
   `unsigned int cmd` local gives `$s1 = cmd`, `$s2 = &D_800BCD80` and no `ret`
   home, i.e. a permuted set that changes the whole function. Pinning `cmd` to
   `$16` and `ret` to `$17` reproduces retail's prologue and the dispatch tree
   exactly, but then cc1 runs out of registers in the first body and spills the
   `D_8009D2C8->f54` load into an early `lui`/`lw` that retail issues after the
   second `addiu $s0,$s0,4`.
2. **First-body pointer walk.** Retail walks `p` with
   `addiu $s0,$s0,4` twice and then `addiu $s0,$s0,8`, loading `ret` and the
   `func_8008CB54` argument as it goes. Writing the same walk in C shapes it
   correctly (`p += 4; ret = *(unsigned short *)p; p += 4; a = *(unsigned short *)p;
   ...; p += 8;`) but the load hoisting above is unaffected by the walk form.
3. **A missing `nop` in the tree.** Retail has a `nop` at `0x8008CC2C` (the
   `0xD8` test's branch-delay slot before the `sltiu $v0,$s0,0xD9`); cc1 always
   produces the `sltiu` there. This is a scheduling difference, not a layout
   one — it persists across `-O2`/`-O1`/`-fschedule-insns2`.

`func-8008CBA8` is not in the `non-C-matchable` class: every instruction is
ordinary compiler output. The residuals are cc1 allocation/scheduling only.

## Best attempt on file

`/tmp/var_G.c` (not committed): `register unsigned int cmd asm("$16")`,
`register int ret asm("$17")`, switch on the `cmd` local, cases ordered
`0x10/0x12/0x19`, `0x24`, `0xD8`, `0xD9`, `0xDA`, `0x99`, `0x98`, default.
Result: `linked .text 928 bytes, target 0x3c8, word mismatches=204`.

## Commands

```
cp /tmp/var_G.c src/func_8008CBA8.c
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  python3 tools/analysis/era_link_check.py src/func_8008CBA8.c 0x8008CBA8 0x3c8 -O2 -G0
# linked .text 928 bytes, target 0x3c8, word mismatches=204, nonzero_pad=0
```

## Reopen only with

A genuinely new lever for either (a) forcing `$s0`/`$s1`/`$s2` to
`&D_800BCD80`/`ret`/`D_800BCD84` simultaneously without spilling the
`D_8009D2C8` base, or (b) a cc1 scheduling form that emits the `nop` in the
`0xD8` test's delay slot.
