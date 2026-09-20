# func_80034FC4

- VRAM 0x80034FC4, file 0x257C4, size 0x74 (29 words), unit 24240.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: 14-slot actor freelist init. Publishes the base as head to
  gp+0x53C (D_8009D2AC = 0x8009D2AC), chains slot 1..13 +4 next-pointers
  to the following slot (stride 0x280), clears the gp halfword/word flags,
  zeroes 16 qwords at D_800A7624 and the D_800C0B14 gate.
- Divergences resolved:
  - D_800BEA94 is the real D_800BEA90+4 symbol. Indexing it through a byte
    offset keeps retail's 3-word `lui %hi / addu / sw %lo` store; a plain
    pointer decayed to register-indirect `sw $a0,0($reg)`.
  - `D_800C0B14` must stay absolute (`lui/sw %lo`) even under -G8, so it is
    declared `extern int D_800C0B14[];`.
  - The 16-entry zero loop is a do-while on a byte offset so cc1 does not
    reverse it into a down-count.
