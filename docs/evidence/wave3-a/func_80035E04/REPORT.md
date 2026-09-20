# func_80035E04

- VRAM 0x80035E04, file 0x26604, size 0x150 (84 words), unit 24240.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: per-actor update. If D_8009D1A0 bit 0x100 is set it republishes
  via func_800361F4. Otherwise snapshots pose +0x28/+0x2C/+0x30 into
  +0x40/+0x44/+0x48 and +0x38/+0x3A/+0x3C halfwords into +0x50/+0x52/+0x54,
  republishes, conditionally integrates +0x88/+0x8C/+0x90 when +0x98 bit 1
  is set, then integrates +0x78/+0x7C/+0x80 and +0x58/+0x5C/+0x60 into pose.
- Divergences resolved: `D_8009D1A0` is loaded absolutely (`lui/lw`) so it is
  declared `extern unsigned int D_8009D1A0[];` to stay absolute under -G8.
