# func_800536B8 — wave7-b executed-path leaf (903 -> 905)

- VRAM 0x800536B8, file 0x43EB8, size 0x174 (93 words). Splat size authoritative.
- Profile: `era_o2_g8` (`-O2 -G8`). Registered in
  `assignments.era_o2_g8`.

## Retail structure
Two-stage signed-16-bit-index record lookup on `D_8009D048[arg0]`.
Stage 1 resolves a secondary record pointer: an inline record when the id is
in 0x100..0x17F (`D_800BEEAC + id*0x20`), else `func_8005DC9C` for ids below
0x100 and for 0x200..0x208 (`D_8009D03C + id - 0x201`). Stage 2 (only while
`arg0` is inside `D_8009D050`) resolves the primary record the same way
(`func_8005DB44` below 0x100, absolute table `D_8009DE64` for 0x200..0x208),
then hands both to `func_800534E4`. Returns the primary record (0 when out of
range).

## Levers
- `D_8009D048` is `extern short *` (the gp-relative pointer scalar at 0x2D8);
  `D_8009D03C` (0x2CC) and `D_8009D050` (0x2E0) are gp scalars.
  `D_800BEEAC` and `D_8009DE64` are declared as incomplete arrays so they stay
  absolute in the `-G8` unit.
- The stage-2 id local must be `int`, not `short`, or cc1 folds the load
  straight into `$a1`.
- Home pins: `a1 asm("$17")` (stage-1 result across the stage-2 calls, fixes
  the s0/s1 swap so arg0 lands in `$16`), `t asm("$5")` (private stage-2 id
  copy), `v1 asm("$3")`, `sh asm("$3")`, `sh1 asm("$2")`.
- The decisive ordering: `t = (void *)(int)v1;` must be issued before the
  0x100 range test so the copy fills that branch delay slot while
  `a0 = v1 - 1` still reads `$3`.

## Authority
`scripts/split_us.sh` (host) then `build_us.sh` + `verify_us.sh` (pe-mipsel):
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (905 registered C leaves)`, plan
`1312 spans = 905 c + 405 asm + 2 rodata`, `VERIFY_US=PASS`. Commit `365014dd`.
