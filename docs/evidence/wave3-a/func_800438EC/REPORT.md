# func_800438EC

- VRAM 0x800438EC, file 0x340EC, size 0xEC (59 words), unit 340EC.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: allocates two nodes (func_80062D2C / func_8006322C), installs the
  func_80043DA4 and func_8004F838 callbacks, runs func_80062CB8, derives a
  9-bit flag count from D_8009CEF0 (mask 0x1F or 0x1EF depending on
  func_8005B89C) for func_800647D0, then clears D_8009CEFC / sets
  D_8009CEF8 and runs the func_800439D8 / func_8004C594 tail.
- Divergences resolved: gp base 0x8009CD70 maps gp+0x180/0x188/0x18C to
  D_8009CEF0/D_8009CEF8/D_8009CEFC, declared as scalars so -G8 emits the
  gp-relative stores retail uses. The popcount loop is `for (i = 8; i >= 0;
  i--)` and its result is the second func_800647D0 argument.
