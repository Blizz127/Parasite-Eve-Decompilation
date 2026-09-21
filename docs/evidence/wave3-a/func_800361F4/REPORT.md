# func_800361F4

- VRAM 0x800361F4, file 0x269F4, size 0x60 (24 words), unit 24240.
- Profile: `era_o2_g8_aspsx_230` (`-O2 -G8`, `ERA_ASPSX_VER=2.30`).
- Fresh build: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  (846 -> 852 registered C leaves).
- Retail: publishes the actor pointer to D_8009D2F0 (gp+0x580), then walks
  the three slots at actor+0xA0; every non-null slot is stored to
  D_8009D300 and handed to func_80017018.
- Divergences resolved:
  - `D_8009D2F0` is gp-relative (scalar) while `D_8009D300` is absolute, so
    the latter is `extern int D_8009D300[];`.
  - `ERA_ASPSX_VER=2.30` removes the nop maspsx inserts between the slot load
    and the `lui/sw %lo` expansion, letting the `lui` fill the load-delay slot
    exactly as retail does.
  - Loop rewritten as `i = 0; p = ...; do { ...; i++; p++; } while (i < 3);`
    so $s0/$s1 land on retail's pointer/counter.
