# agent-decomp105 — tooling/model levers for the slice-6 remainder

Worktree `/tmp/pe-agent-decomp105`, branch `agent/decomp105`, base `d2ae6879`.
Task: crack the two levers parked by `agent-decomp102` — (1) `$gp`-relative
readers and (2) return (`jr $ra`) delay slots filled with the epilogue stack
restore — and demonstrate each with real matching leaves.

## Baseline gate (reproduced before any change)

- `bash scripts/split_us.sh` → `c: 724 split`.
- `bash scripts/build_us.sh` → `Compare: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
  `Matching claim: YES (724 registered C leaves)`.
- `bash scripts/verify_us.sh` → `VERIFY_US=PASS`.

## Result

**+8 newly matched C leaves: 724 → 732.**
Final fresh build: `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (732 registered C leaves)`; `verify_us.sh` → `VERIFY_US=PASS`
(plan `1060 spans = 732 c + 326 asm + 2 rodata`).

## Lever 1 — `$gp`-relative readers: **already cracked; recipe documented**

**The task premise is out of date.** 88 of the 724 pre-existing matched leaves
already emit `R_MIPS_GPREL16`; after this pass 91 of 732 do. `func_800124F8`
(`era_o2_g8`) is the canonical example: its retail words are
`sw $zero,0x590($gp)`, `sh $zero,0x598($gp)`, `sw $zero,0x8C($gp)`,
`sw $zero,0x90($gp)`, `sw $zero,0x94($gp)`, and the built object resolves them
as `R_MIPS_GPREL16 D_8009D300 / D_8009D308 / D_8009CDFC / D_8009CE00 / D_8009CE04`.

### The mechanism (three ordinary pieces, no new tooling)

1. **cc1 `-G<n>`** (the `era_o2_g8` / `modern_o1_g8` profiles) makes cc1 emit
   `.extern D_8009CE00, 4` next to each scalar `extern` reference whose declared
   object size is ≤ `n`. At `-G0` cc1 emits no `.extern` at all, which is why
   `-G0` leaves never go gp-relative.
2. **maspsx leaves the bare `lw/sw $r,SYM` line alone.** The maspsx driver is
   invoked without any `-G`, so `sdata_limit == 0` and both gp paths
   (`tools/era/maspsx/maspsx/__init__.py`, load branch ~L1046 / store branch
   ~L1178) pass the line through unchanged. (This is also why the gp lever needs
   no maspsx patch: maspsx is not the component that decides.)
3. **GNU as defaults to `-G8`.** `mipsel-linux-gnu-as` is invoked with only
   `-EL -mips1 -mabi=32`; its default MIPS small-data limit is 8, so every
   `.extern SYM,<size≤8>` reference is assembled as `op $r,%gp_rel(SYM)($gp)`.

The linker then resolves `%gp_rel` against `_gp`. `tools/build/disc1_build.py`
sets `_gp = 0x8009CD70`, matching the retail crt0:

```
/* 62DB0 800725B0 */  lui   $gp, %hi(D_8009CD70)
/* 62DB4 800725B4 */  addiu $gp, $gp, %lo(D_8009CD70)
```

**Correction to the task note:** the retail `$gp` base is **`0x8009CD70`**, not
`0x8009CD60` (off by `0x10`). Consequently `0x590($gp)` = **`D_8009D300`**, not
`D_8009D2F0`; `D_8009D2F0` is a large/absolute symbol (its readers use
`lui`/`lw %lo(D_8009D2F0)`). Every gp offset observed maps to
`D_8009<0x8009CD70+off>` and those names were already referenced by other
matched leaves (`0x90→D_8009CE00`, `0x2E8→D_8009D058`, `0xCC→D_8009CE3C`,
`0x590→D_8009D300`).

### Selective control: small vs absolute within one function

`-G8` makes *every* scalar `extern` gp-relative, but retail sometimes reads the
same-size symbol absolutely in the same function (`func_80021054`: `D_8009D278`
absolute via `lui/lw`, `D_8009CE3C` gp via `lb 0xCC($gp)`). The lever is the
**declared type size**:

- scalar/known small object (`extern unsigned int *D_8009D300;`,
  `extern signed char D_8009CE3C;`) → cc1 emits `.extern …,4/1` → gp-relative.
- incomplete-array object (`extern int *D_8009D278[];`) → cc1 emits **no**
  `.extern <size>` → GNU as leaves the load absolute. (This is the same idiom
  already used by `src/func_800124F8.c` for `D_8009D310[]`/`D_8009DF70[]`.)

The typing must also match retail's opcode: `lb` vs `lbu` is `signed char` vs
`unsigned char` (`func_80021054` needs `signed char`).

### New gp leaves (all `era_o2_g8`)

| # | leaf | VRAM | file | size | gp operand | note |
|---|------|------|-----:|-----:|-----------|------|
| 1 | `func_8001784C` | 0x8001784C | 0x804C | 0x30 | `0x590($gp)` = D_8009D300 (×2) | `extern unsigned int *D_8009D300;` |
| 2 | `func_80021054` | 0x80021054 | 0x11854 | 0x2C | `0xCC($gp)` = D_8009CE3C | `D_8009D278[]` incomplete array keeps the header absolute; `signed char` gives `lb` |
| 3 | `func_80055FE0` | 0x80055FE0 | 0x467E0 | 0x2C | `0x2E8($gp)` = D_8009D058 | signed `1 << (index & 0x1F)` (unsigned `1u` makes cc1 fold to `srlv`/`andi 1`) |

Verified by `tools/analysis/era_leaf_match.sh`: each candidate differs from retail
**only** in the unresolved relocation immediates (`HI16/LO16` and `GPREL16`),
which the link resolves. `build_us.sh` confirms byte-exact.

## Lever 2 — return delay slot filled with the stack restore: **cracked (maspsx patch)**

cc1 2.7.2 emits the epilogue deallocation *before* the return jump and no nop
after it; it relies on the assembler's delay-slot scheduler. ASPSX hoists the
restore into the slot, giving retail's shape:

```
lw   $31,0x14($sp)
lw   $16,0x10($sp)
jr   $31
addiu $sp,$sp,0x18
```

maspsx appended a `nop` slot instead, so the only divergence for
`func_80075B4C`/`75C04`/`7DD74` was the order of the last two words.

### Patch (opt-in env `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`)

`tools/era/maspsx/maspsx/__init__.py`: a new parser branch (before the generic
`addu` case) detects an `addu`/`addiu` with operands `$sp,$sp,<imm>` immediately
before a bare `j $31` (`_is_stack_restore` + `_next_line_is_return_jump`), emits
`j $31` followed by the restore, and consumes the jump (`skip_instructions = 1`)
so no nop slot is appended. It is a pure reorder — no address synthesis — and is
strictly guarded by the constructor/env flag, which defaults OFF.

House rules honoured:
- Default (env unset, no profile) is byte-identical: the new branch is the only
  behavioural change and it cannot fire while `fill_epilogue_delay_slot` is
  False; the full maspsx suite (now **180** tests) passes, including the
  flag-off regression cases in the new `tests/test_fill_epilogue_delay_slot.py`
  (`test_default_is_off`, `test_non_stack_addu_is_untouched`,
  `test_label_blocks_fill`).
- The full 732-leaf EXACT rebuild proves every pre-existing leaf still compiles
  byte-identically through the patched maspsx (if the default path had moved,
  the 724 unchanged spans could not all still match).
- New test tracked via `.gitignore` negation + `setup_era.sh` `MASPSX_TRACKED`
  entry, matching the prior local-patch files.

New profile `era_o2_g0_fill_epilogue_delay_slot` (`-O2 -G0` +
`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`).

### New epilogue-slot leaves

| # | leaf | VRAM | file | size | shape |
|---|------|------|-----:|-----:|-------|
| 1 | `func_80075B4C` | 0x80075B4C | 0x6634C | 0x38 | marker byte + `func_800762BC(arg1)`, store v / 0 |
| 2 | `func_80075C04` | 0x80075C04 | 0x66404 | 0x40 | marker byte + `func_800762A0(a1[0],a1[1])`, store v / 0 |
| 3 | `func_8007DD74` | 0x8007DD74 | 0x6E574 | 0x34 | `func_8007DDC4(a0); func_8007DDB4(a0,0x3F,0)` |
| 4 | `func_800755BC` | 0x800755BC | 0x65DBC | 0x34 | `func_80071A34(a0,D_8009575C,0x5C); return a0` |
| 5 | `func_80075AE8` | 0x80075AE8 | 0x662E8 | 0x34 | `func_80071A34(a0,D_800957B8,0x14); return a0` (twin of 755BC) |

### Lever scope

A worklist scan finds **202** remaining functions whose `jr $ra` delay slot is
`addiu $sp,$sp,N` (the `MASPSX_FILL_EPILOGUE_DELAY_SLOT` family) — the lever
generalises well beyond the five demonstration leaves.

## Residual / not cracked (negative results)

- `func_8005E4E4` / `func_8005E518` (0x4ECE4 / 0x4ED18, 0x34 each) combine the
  epilogue slot with a gp load (`0x378($gp)` = D_8009D0E8) and are byte-close but
  not matched:
  - `func_8005E4E4`: retail `andi $v0,$v0,0x20 / sltu $v1,$zero,$v0`; cc1 2.7.2
    strength-reduces the single-bit test to `srl $v1,$v0,5 / andi $v1,$v1,1`.
    Tried unsigned/signed mask, `== 0x20`, named locals, `-O1` — the bit-test
    fold is invariant.
  - `func_8005E518` (mask `0x5000`, not a single bit): only a register choice
    differs — retail `andi $v0,…` then `sltu $v1,$zero,$v0`, cc1
    `andi $v1,…` then `sltu $v1,$zero,$v1`.
  Both remain parked; the gp half of their divergence is not the blocker.
- The gp lever needs **no** new maspsx change; the earlier park note's
  "separate gp-base/model investigation" was unnecessary — the model was already
  present and in use.

## Reproduce

```
cd /tmp/pe-agent-decomp105
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp105 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp105 && bash scripts/verify_us.sh'
python3 -m pytest tools/era/maspsx/tests -q
```
