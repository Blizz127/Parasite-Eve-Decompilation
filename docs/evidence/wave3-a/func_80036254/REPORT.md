# func_80036254

- VRAM 0x80036254, file 0x26A54, size 0x64 (25 words), unit 24240.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: walks the three task lists anchored at actor+0xA0. For each node
  with a non-null +0x04 link it re-points +0x00 at it, sets +0x10 to 1 and
  clears bits 5-6 of the +0x08 halfword, following the +0x24 next links.
- Divergences resolved: struct layout only; the nested `for`/`while` produced
  retail's outer counter loop and inner while directly.
