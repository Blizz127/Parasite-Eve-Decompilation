# func_800C3238

- **VRAM**: 0x800C3238
- **File offset**: 0xB3A38 (size 0xEC)
- **Unit**: B3934
- **Build profile**: era_o2_g0_dispatch_800c2128 (`-O2 -G0` +
  `MASPSX_THREE_WORD_SYMBOL_STORE=1` + `MASPSX_DISPATCH_FOLD=jtbl_800C2128`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
Selector-byte dispatcher. Stores arg0 into D_800F33B8, then maps
`arg0 & 0xFF` 0..4 onto the D_800F337A / D_800E224C pair (0/0, 1/0, 1/1, 1/2,
1/3); other selectors leave both untouched. Finally computes
func_80077A64(D_800F33AC, D_800E224C, D_800F3424, D_800F3426) and stores the
halfword result in D_800E27AC.

## Method
A plain `switch (arg0 & 0xFF)` with the five cases reproduces retail's
`sltiu $v0,$a0,5` guard and jump-table dispatch. cc1 emits the compound
dispatch `lw $2,$L8($2)`; maspsx only expands it to retail's three-word
`lui $at,%hi(jtbl); addu $at,$at,$v0; lw $v0,%lo(jtbl)($at)` when **both**
gates are on — `MASPSX_THREE_WORD_SYMBOL_STORE=1` and
`MASPSX_DISPATCH_FOLD=jtbl_800C2128`. With either off, the expansion is four
words and every branch offset shifts. A new profile
`era_o2_g0_dispatch_800c2128` was registered for the leaf.

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (876
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
