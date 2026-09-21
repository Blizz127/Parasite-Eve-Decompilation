# func_8003E974 — wave7-a

- **VRAM** 0x8003E974, **file** 0x2F174, **size** 0x154 (splat).
- **Profile** `era_o2_g8_three_word_force_d8009d1a0_absolute`
  (`-O2 -G8` + `MASPSX_THREE_WORD_SYMBOL_STORE=1` +
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0`); assigned in
  `configs/USA/disc1_build_profiles.json`.
- **Source** `src/func_8003E974.c`.

## What it is

SPU/voice mask reset. Clears the five gp-resident state words
`D_8009D1E4/D1F4/D2D4/D238/D26C`, zeroes the 0x80-byte `D_800A76F0` array,
re-registers the twenty-two register-window pairs through `func_8003EAC8`,
then sets bit 0x4000 in `D_8009D1A0` and returns the new value.

## Levers

- The five state words are gp-relative and the loop `sltiu` is unsigned, so
  the unit is `-O2 -G8`; the calls need no other gate.
- `D_8009D1A0` is at gp+0x430 but retail addresses it **absolutely**
  (`lui v0,%hi; lw v0,%lo; ...; lui at,%hi; sw v0,%lo(at)`). It is declared as
  a plain scalar `extern unsigned int D_8009D1A0;` and the profile's
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS` strips cc1's `.extern` so maspsx emits the
  absolute form. Declaring it an incomplete array instead produces
  `la $r` + `0($r)`, which is the wrong shape.
- With force-absolute *and* `MASPSX_THREE_WORD_SYMBOL_STORE=1` the leaf also
  matches, so the pre-existing
  `era_o2_g8_three_word_force_d8009d1a0_absolute` profile was reused (its
  three-word gate does not fire here) rather than adding a near-duplicate
  profile.

`try_leaf` before the carve: `WORDS MATCH (+12 pad bytes, trimmed by the
build)`.

## Authority

Fresh `scripts/split_us.sh` (host) + `scripts/build_us.sh` +
`scripts/verify_us.sh` (pe-mipsel) on the carved tree:
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (904 registered C leaves)`, plan
`1312 spans = 904 c + 406 asm + 2 rodata`, `VERIFY_US=PASS`.
Commit `88790708`.
