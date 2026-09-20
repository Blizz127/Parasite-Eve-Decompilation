# 60C1C RNG trio — `MASPSX_EXPAND_LI=1` retry (agent/rng-ori)

Base: `8db69a00` (per-leaf `MASPSX_EXPAND_LI` gate). Baseline re-verified before
and after this retry: **EXACT SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`**,
`Matching claim: YES (866 registered C leaves)`, `VERIFY_US=PASS`. No leaf
landed, no YAML/profile change: the matching count stays **866**.

## Verdict

The li gate **was necessary but is not sufficient** for this trio. It removes
exactly one of the park note's blockers — the scalar `li`→`ori` immediate form —
and that only shows up in the `ori $r,$zero,imm` words. The three functions are
still held by independent, gate-immune residuals: absolute-symbol store macros,
`$t`-register home plans, and ASPSX-style pre-decrement branch scheduling.

## func_80070D10 (file 0x61510, 0x5C, 23 words)

With `MASPSX_EXPAND_LI=1` a literal base `(int *)0x80070E0C` does compile to the
retail `lui/ori` pair and the scalar words compile to `ori $r,$zero,imm`. The
explicit `goto`-rotation (`body: … ; if (t5) { t5--; goto body; }`) also removes
the park note's `base+0x3C` bias: cc1 then keeps the base un-biased and emits
`lw $3,0x40($4)` / `lw $2,0x3C($4)` / `sw $2,0x3C($4)` — retail offsets. Three
residuals remain:

1. **Store macros.** Both seed stores and both index stores compile as the
   absolute-symbol store macro `sw $r,SYM+off` → `lui $at,%hi; sw $r,%lo($at)`
   (two words each). cc1 rematerialises the constant base; retail keeps it in
   `$t0` and pays one-word `sw $t3,0x40($t0)` / `sw $t3,0(t1)`. No source shape
   tried (extern symbol, extern array, `volatile` pointer, `char *` byte
   offsets, two pointers, goto rotation) makes cc1 use base+offset here.
2. **Counter canonicalisation.** `do{…}while(t5--)`, `if(t5--)goto body`, and
   `if(t5){t5--;goto body}` all canonicalise to `addiu $5,$5,-1; bne $5,-1`
   (or a reversed `beq`/`j` pair). Retail is `bnez $5,.L` with
   `addiu $5,$5,-1` in the branch delay slot (test on the pre-decrement value).
3. **Register homes.** Retail runs the whole leaf in `t0/t3/t4/t5`; cc1 assigns
   the base/values/counter to `$4/$3/$2/$5`. Also `addu $4,$4,-4` vs retail
   `addiu $0,$0,-4`.

Closest candidates: `func_80070D10_closest_literal_goto.c` (literal base, goto
rotation; retail 92 B vs candidate 112 B) and an extern-symbol form (13 word
diffs, but only because `try_leaf` zeroes the `la` HI16/LO16 words — the
resolved build still has the wrong base registers and store macros).

## func_80070D6C (file 0x6156C, 0x64, 25 words)

The gate fixes only the two `index = 0x40` immediates (`ori $t1,$zero,0x40`,
`ori $t2,$t2,0x40`). The register-home / cross-block-scheduling residual from
`docs/evidence/volume-campaign-20260830/func-80070d6c/PARK.md` is unchanged:
the literal-base candidate is 24 words (retail 25) with the three bases in
`v0/t0/a3` instead of `t0/t7/t8`, the sum computed directly in `$v0` (retail's
`or v0,zero,t5` absent), no explicit load-delay `nop`, and the two guard stores
scheduled differently. Files: `func_80070D6C_closest_literal.c` (25 diffs,
all words compared) and `func_80070D6C_closest_extern.c` (17-19 diffs at
`-O2/-G8/-O1` × ASPSX 2.21/2.30 after `la` zeroing).

## func_80070DD0 (file 0x615D0, 0x34, 13 words)

**Handwritten, parked immediately.** It preserves `$ra` across its
`jal func_80070D6C` in caller-saved `$v1` (`or $v1,$zero,$ra` before the call,
`or $ra,$zero,$v1` after the `mult`) with **no stack frame**. No C compiler
keeps `$ra` across a call without a frame save; GCC 2.7.2 emits
`sw $ra,N($sp)`. spimdisasm independently labels the function
“Handwritten function” and the trailing `sub $a1,$a1,$a0` /
`add $v0,$v0,$a0` as handwritten instructions. The 16.16
`mult`/`mflo`/`mfhi`/`srl`/`sll`/`or` recombine is C-expressible; the `$ra`
home is not, so no C retry is possible.

## Reproduction

```sh
# baseline
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-rng && bash scripts/build_us.sh && bash scripts/verify_us.sh'

# triage (gate on), e.g.
python3 tools/analysis/try_leaf.py docs/evidence/rng-ori/func_80070D10_closest_literal_goto.c \
    0x61510 0x5C --flags "-O2 -G0" --env MASPSX_EXPAND_LI=1
python3 tools/analysis/try_leaf.py docs/evidence/rng-ori/func_80070D6C_closest_literal.c \
    0x6156C 0x64 --flags "-O2 -G0" --env MASPSX_EXPAND_LI=1
```

Profiles swept for both leaves: `-O1/-O2 × -G0/-G8 × ASPSX 2.21/2.30` plus
`-fno-gcse`, `-fno-strength-reduce`, `-fno-expensive-optimizations`,
`-fno-schedule-insns2`, `-fno-caller-saves` — best remained 13-23 differing
words and never zero.
