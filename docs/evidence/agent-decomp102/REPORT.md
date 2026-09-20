# agent-decomp102 — small-function sweep, slice 6 (ranks 753-1680)

Worktree `/tmp/pe-agent-decomp102`, branch `agent/decomp102`, base `58ffdd4b`.

## Baseline gate (reproduced before any change)

- `bash scripts/split_us.sh` → `c: 709 split`.
- `bash scripts/build_us.sh` → `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
  `Matching claim: YES (709 registered C leaves)`.
- `bash scripts/verify_us.sh` → `VERIFY_US=PASS`.

## Result

**+14 newly matched C leaves: 709 → 723.**
Final fresh build: `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (723 registered C leaves)`; `verify_us.sh` → `VERIFY_US=PASS`
(plan `1046 spans = 723 c + 321 asm + 2 rodata`).

Commit range: `58ffdd4b..4986f3e8` (14 leaf commits + 2 generated-status doc commits).

## Matched leaves

| # | leaf | VRAM | file off | size | lever | commit |
|---|------|------|---------:|-----:|-------|--------|
| 1 | `func_80017E68` | 0x80017E68 | 0x8668 | 0x34 | era_o2_g0_aspsx_230; mask-subset predicate `*(a0[1]) = ((state[0x98] & *a0[0]) == *a0[0])` | `b8aaf829` |
| 2 | `func_80018E84` | 0x80018E84 | 0x9684 | 0x30 | **era_o1_g0_aspsx_230**; `volatile` pointee keeps the wide `lw` (otherwise cc1 narrows to `lhu`) and `-O1` keeps retail's store order | `db18e2e7` |
| 3 | `func_80019260` | 0x80019260 | 0x9A60 | 0x38 | era_o2_g0_aspsx_230; call + state clear, `addu $sp` in the `jr` slot | `e2e4ba1a` |
| 4 | `func_800196E8` | 0x800196E8 | 0x9EE8 | 0x40 | era_o2_g0_aspsx_230; `(short *)(state->0x9C) + *a0[0]`, halfword commit | `d5455820` |
| 5 | `func_80019CEC` | 0x80019CEC | 0xA4EC | 0x38 | era_o2_g0_aspsx_230; **source order is the lever**: writing 0x21C/0x21E/0x220 (not 21C/220/21E) makes cc1 emit retail's load order | `73aa5c5e` |
| 6 | `func_80019DB8` | 0x80019DB8 | 0xA5B8 | 0x3C | era_o2_g0_aspsx_230; 12-byte record table index (`*3 <<2`) | `98efc1ef` |
| 7 | `func_8001A374` | 0x8001A374 | 0xAB74 | 0x1C | era_o2_g0_aspsx_230; byte store of a double-deref word | `3d6856ce` |
| 8 | `func_8001A3FC` | 0x8001A3FC | 0xABFC | 0x40 | era_o2_g0_aspsx_230; `(char*)((void**)state)[0x9C/4] + *a0[0]*2`; `$v1` index pin | `f1ada524` |
| 9 | `func_80073D24` | 0x80073D24 | 0x64524 | 0x34 | era_o2_g0_aspsx_230; indirect call `D_8009566C->0x14(4, arg0)` | `8ec4feaf` |
| 10 | `func_80090AAC` | 0x80090AAC | 0x812AC | 0x40 | era_o2_g0_aspsx_230; two-call wrapper with s0/s1 frame | `fd0609fd` |
| 11 | `func_80065A9C` | 0x80065A9C | 0x5629C | 0x38 | era_o2_g0_aspsx_230; **`volatile` header pointer** reproduces retail's double `D_800B1624` load; `*p \|= arg1 & 0x30` | `8df1837d` |
| 12 | `func_8006599C` | 0x8006599C | 0x5619C | 0x2C | era_o2_g0_aspsx_230; volatile header; signed halfword at +6 | `1b2c1d45` |
| 13 | `func_800659C8` | 0x800659C8 | 0x561C8 | 0x30 | era_o2_g0_aspsx_230; volatile header; `arg1 >> 8` at +8 | `9b667029` |
| 14 | `func_8006E6A8` | 0x8006E6A8 | 0x5EEA8 | 0x2C | era_o2_g0_aspsx_230; `func_8006E6D4(arg0, 0, arg1, arg2)` | `554dbc3d` |

New build-profile assignments added to `configs/USA/disc1_build_profiles.json`:
`era_o1_g0_aspsx_230` (new profile: `-O1 -G0` + `ERA_ASPSX_VER=2.30`) and 13 leaves
onto the existing `era_o2_g0_aspsx_230`.

### Levers found

1. **`ERA_ASPSX_VER=2.30` is the default-era correction for small symbolic
   stores.** ASPSX < 2.30 inserts a `nop` before a symbol-store macro's `$at`
   expansion (`nop_at_expansion=True` in maspsx). Retail's small store macros
   in this region have no such nop and put the `lui $at` straight after the
   producing load; selecting 2.30 removes the injected nop and matches. 13 of
   the 14 leaves run on `era_o2_g0_aspsx_230`.
2. **`volatile` on a loaded structure pointer is a scheduling/width lever.**
   `func_80065A9C`/`6599C`/`659C8` need the header global loaded *twice* (once
   for the base, once for the `0x10` field); only a `volatile` pointer stops
   cc1's CSE. `func_80018E84` needs `volatile` on the pointee to keep a full
   `lw` where cc1 would otherwise narrow to `lhu` for a truncating store.
3. **Source statement order selects the load schedule** for independent
   halfword reads (`func_80019CEC`): the natural order produced 0x21C/0x21E/0x220;
   swapping the last two statements made cc1 emit retail's 0x21C/0x220/0x21E.
4. `register ... asm("$N")` pins still fix register-allocation mismatches
   (`func_8001A3FC` index in `$v1`).

## Parked / not attempted further (two-attempt rule)

Instruction-level divergence for each; all park without touching the tree.

| leaf | file/size | divergence |
|------|-----------|------------|
| `func_8005DB8C` | 0x4E38C / 0x20 | retail `sll a0,9` + `addiu v1,v0,-0x10` + `lw v0,0(v0)`; cc1 materialises the symbol with the `lw $r,SYM` `$at` macro and reassociates `(load+shift)+(base-16)` — 2-3 words differ. Considered a new profile/address-form lever. |
| `func_80075B4C` | 0x6634C / 0x38 | retail fills the `jr $ra` delay slot with `addiu $sp,$sp,0x18`; cc1 emits `lw $16; addu $sp; j $31` (teardown before the jump). Same for `func_8007DD74` (0x6E574) and `func_80075C04` (0x66404). |
| `func_8007FCBC` | 0x704BC / 0x40 | retail `bgtz` delay slot is `addu $v0,$zero,$zero` (the shared `return 0`) and the base lives in `$v1` (`sw $v0,-0x28($v1)`); cc1 hoists the load before `subu $sp` and uses a symbolic store. |
| `func_80065A60` | 0x56260 / 0x3C | after pinning the final pointer to `$a1`, 1 word remains: retail `addu $a1,$a1,$v0` vs cc1 `addu $a1,$v0,$a1` (operand canonicalisation). |
| `func_80082ADC` | 0x732DC / 0x2C | retail materialises `&D_800A5AB4` once in `$v0` and stores `sw $v1,off($v0)`; cc1 folds every store to the `sw $r,SYM` `$at` macro. |
| `func_800CC244` / `func_800CC284` | 0xBCA44 / 0xBCA84 | store/load schedule: retail loads `D_800E2294` before the `0xBE8`/`0x7F` constants; cc1 defers it. `func_800CD07C` (0xBD87C) analogous. |
| `func_8006A2E8` | 0x5AAE8 / 0x30 | retail copies `$a1` to `$v0` (`addu`) then stores `$v0` three times; cc1 stores `$a1` directly (1 short) and branch span differs. |
| `func_80084F8C` | 0x7578C / 0x2C | retail `beq $v1,$v0` with `$v0=0xFF` load; both source phrasings produced `sltu`/`xori` shapes. |
| `func_8007E594` | 0x6ED94 / 0x30 | retail strength-reduces the 5-byte clear to `$v0 = a0+3` + `sb 5($v0)` + `addiu $v0,-1`; cc1 re-computes `$v0 = $v1 + $a0`. |
| `func_8008227C` | 0x72A7C / 0x30 | retail schedules the `la` low half into the `jal` delay slot (`lui $a0,...` / `jal` / `addiu $a0,...`) and uses `addiu $sp`; cc1 emits `la` then `jal`+`nop` and `subu`. |
| `func_8007FB04` | 0x70304 / 0x40 | retail puts `sw $zero,D_8009B554` in the delay slot of the first `jal` (with `lui $at` before it); cc1 emits the store before the call and a `nop` in the slot. |
| `func_80087050` / `func_8007E160` / `func_8008F430` | 0x77850 / 0x6E960 / 0x7FC30 | loop / short-circuit shapes and wide-load ordering; 9-15 words differ, no lever found in two attempts. |
| `func_80067B40` | 0x58340 / 0x34 | cc1 hoists the `-0xC01` mask materialisation and swaps `$v0`/`$v1`/`$a0` roles. |
| `func_80075C44` | 0x66444 / 0x28 | retail emits `li $v0,2; sb $v0,3(a0)` before the `beqz` whose delay slot is `lui $v1,0xE600`; cc1 hoists the `lui`. |
| `func_8003E0A4` | 0x2E8A4 / 0x2C | retail keeps the 1/3 selector in `$v0`; cc1 allocated `$a3` (pinning `$2` still 12 words differ). |
| `func_80056C14` | 0x47414 / 0x2C | indexed `lhu %lo(D_800A1E6E)($at+idx)` with a `j` to the shared return; cc1 shape differs. |
| `func_80071964` / `func_80071994` | 0x62164 / 0x62194 | retail `j .L; addu $v0,$v0,$v1` delay-slot add; cc1 emits a straight-line add. |
| `func_80083578` | 0x73D78 / 0x28 | retail places the load-delay `nop` *before* the loop label; cc1 puts the label before the `lhu`, so maspsx adds a per-iteration nop (`beqz` offset -5 vs -4). |
| `func_80078C94` / `func_80087798` | 0x69494 / 0x77F98 | 3-word copy and the 0x1F801C00 MMIO pair; cc1 schedules `$at`/`$v0` differently (8-10 words). |
| `func_80072714` / `func_80072724` | 0x62F14 / 0x62F24 | `addiu $a0,$zero,1/2; syscall 0` BIOS stubs — not C-expressible. |
| `func_8003708C` / `func_800370BC` / `func_800370CC` | 0x2788C / 0x278BC / 0x278CC | fixed-point helpers; retail `ori $at,$zero,0x8000` + `add`; cc1 uses `$v0` + `addu`. |
| `func_80078120` | 0x68920 / 0x14 | branches to 0x68970, outside the `nonmatching` extent (truncated region) — skip. |
| `func_8007E3B4` | 0x6EBB4 / 0x24 | `jr $v0` computed jump into rodata — not a function body. |
| 4-byte `nop` "functions" (`func_8003E60C`, `func_80089F50`, …) | — | single pad `nop`; worklist false positives, not C leaves. |
| `func_800199CC` | 0xA1CC / 0x2C | cc1 loads the `D_8009D2F0` header before `*a0[0]` and reorders the two dependent `lw`s; retail order 0x00/0x0C. |

The remaining slice-6 candidates not listed above were not started (bounded
sweep; stop rule hit after the 14th match plus a long park run).

## Reproduce

```
cd /tmp/pe-agent-decomp102
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp102 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp102 && bash scripts/verify_us.sh'
```
