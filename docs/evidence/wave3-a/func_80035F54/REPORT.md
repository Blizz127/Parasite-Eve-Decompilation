# func_80035F54

- VRAM 0x80035F54, file 0x26754, size 0xC8 (50 words), unit 24240.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: recursive actor contact-pose restore. When actor+0x18C is set it
  recurses on that link, then walks the D_8009D20C list (gp+0x49C) and
  restores every entry whose +0x18C matches this actor's. When the link is
  null it restores the actor itself and sets bit 0x40000 in +0x98.
- Divergences resolved: none beyond the struct layout; the `for (p = head;
  p != 0; p = p->next)` form reproduced retail's test-body-test walk exactly.
