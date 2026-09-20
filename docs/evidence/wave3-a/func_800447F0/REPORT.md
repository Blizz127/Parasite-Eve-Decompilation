# func_800447F0

- VRAM 0x800447F0, file 0x34FF0, size 0x134 (77 words), unit 340EC.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 853 registered C leaves).
- Retail: menu row/scroll update. Reads func_80062A34(2,1) and the
  func_8005DA8C(1) cursor record, calls func_80063158(act, 0, ...) with the
  ((state+0x38 << 4) + 4) biased cursor, then re-anchors the cursor when
  (state+0x58 - state+0x38) - state+0x5C is 0 or 1, and paints five
  func_8005E8A4/func_8005FA3C rows.
- Divergences resolved: the 0/1 case body MUST be a `switch`, not an
  if/else-if chain; the if-chain inverted the branch polarity and laid the
  case-0 block out as fall-through, while the switch produced retail's exact
  `beqz v1 / beq v1,1` decision chain.
