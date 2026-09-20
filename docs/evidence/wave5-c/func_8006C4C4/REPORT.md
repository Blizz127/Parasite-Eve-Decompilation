# func_8006C4C4

- **VRAM**: 0x8006C4C4
- **File offset**: 0x5CCC4 (size 0xF8)
- **Unit**: 5C6CC
- **Build profile**: era_o2_g0 (`-O2 -G0`, default)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
Frame/voice state setter. `var_a1 = arg0`; when `arg0 == -1` it reloads
D_800B0CE5 into D_800B0CE4, sign-extends it into the new index and ORs 3 into
D_800B0CE6. Then, if D_8009D1A0 bit 1 or D_800B0CD8[0] bit 1 is set, ORs 2
into D_800B0CE6 and clears bit 1 of D_8009D2E8. Clears bit 2 of D_800B0CD8[0xE]
when set, and when the index is in 1..8 and differs from D_800B0CD8[0xD]
records it in [0xD]/[0xC] and sets D_800B0CD8[0xE] bit 0. Returns 0.

## Method
Two cc1 levers on top of the m2c draft:
- The CE5 byte must be read once into a temp (`temp_a0 = D_800B0CE5;
  D_800B0CE4 = temp_a0; var_a1 = (signed char)temp_a0;`) — the raw shift/`lb`
  variants make cc1 allocate an 8-byte phantom frame.
- The CE6 update must be the compound `D_800B0CE6 |= 3;`, not
  `temp_v0 = D_800B0CE6 | 3; D_800B0CE6 = temp_v0;`. The temp form swaps
  `$v0`/`$a0` (CE5 byte in $v0, CE6 result in $a0) and leaves 6 words
  diverging; the compound form colours them exactly like retail ($a0 byte,
  $v0 result).

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (876
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
