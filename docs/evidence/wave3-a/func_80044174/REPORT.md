# func_80044174

- VRAM 0x80044174, file 0x34974, size 0x100 (64 words), unit 340EC.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (853 -> 854 registered C leaves).
- Retail: allocates the help/menu node pair. 62D2C(0x1B) then
  62D2C(1, arg0) with func_800447F0 in n1+0x30; 6322C(1, n2, n2) with
  func_80044444 in n2+0x2C and func_8004F8D0 in n3+0x30; installs
  func_80057C54/func_80050260 at n3+0x84/+0x88; runs func_80055760 and
  func_800647D0(n3, func_80052F70()); stores -1 into gp+0x224/+0x21C and,
  when gp+0x228 is non-zero, decomposes v-1 into n3+0x44/+0x48/+0x5C.
- Divergences resolved:
  - `n1->f30 = func_800447F0;` must be written BEFORE the second
    `func_80062D2C` call, so cc1 can place it in that call's delay slot and
    does not have to keep n1 live in a saved register.
  - The final `(v - 1) >> 8` MUST have its destination pinned to `$v1`
    (`register int hi asm("$3");`). cc1 otherwise reuses `$v0` after the
    f48 store; retail's `sra $v1,$v1,8` is in-place. This is the only
    source-level difference and the only use of a register pin in this
    slice.
