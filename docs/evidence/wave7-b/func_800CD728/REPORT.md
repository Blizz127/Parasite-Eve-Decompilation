# func_800CD728 — wave7-b executed-path leaf (903 -> 905)

- VRAM 0x800CD728, file 0xBDF28, size 0x174 (93 words). Splat size authoritative.
- Profile: default `era_o2_g0` (`-O2 -G0`), no maspsx gates.

## Retail structure
Calls `func_800C22F8()` and stores `&D_800E0F6C` through the returned pointer,
then writes ~31 width-typed constants (int/short/char) to D_800F33xx and
D_800E27xx.

## Levers
- The 0x20 literal must live in `$a0`: `register int z asm("$4"); z = 0x20;`
  as a **separate assignment** (the initializer form is dead-code eliminated),
  with the three `D_800F340D/09/0A = z` stores placed right after
  `D_800F340C = 0x40`, so cc1 emits `li $3,0x40; li $4,0x20; sb $3,D_800F340C`
  exactly and reuses `$a0` for the later byte stores.
- The three `D_800F33F0/2/4 = 0` stores must be issued at the TOP of the
  function (right after the `func_800C22F8` store). The scheduler then lands
  them in retail's slots (after `li $3,0x80`, before the `$a0` stores) instead
  of before them.

## Verification note (important)
An earlier variant passed `try_leaf` with 0 diffs but failed the LINKED build:
try_leaf zeroes every relocation-bearing word, so it hides store ORDER for
absolute symbol stores. The final source was checked with a strict
relocation-resolved compare (0 differing words) and, decisively, by the fresh
linked build.

## Authority
`scripts/split_us.sh` (host) then `build_us.sh` + `verify_us.sh` (pe-mipsel):
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (905 registered C leaves)`, plan
`1312 spans = 905 c + 405 asm + 2 rodata`, `VERIFY_US=PASS`. Commit `365014dd`.
