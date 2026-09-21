# func_8007DD74 — MATCHED (13 words, LINK_EXACT)

VRAM `0x8007DD74`, file `0x6E574`, span `0x34`. Carve: splits `[0x6E538, asm]`;
`func_8007DDA8` (a `nop`-only 4-byte filler) starts at `0x6E5A8`.

```c
void func_8007DD74(unsigned char *a0) {
    func_8007DDC4(a0);
    func_8007DDB4(a0, 0x3F, 0);
}
```

**Lever — maspsx patch 3.** The function's `addiu $sp,$sp,0x18` teardown is
scheduled into the `jr $ra` delay slot, which plain gas emits before the jump
(2 word mismatches). The leaf therefore uses profile
`era_o2_g0_fill_epilogue_delay_slot` (`MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`),
registered in `disc1_build_profiles.json`.

Gate: `LINK_EXACT` with `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`; deep span exact
(769 c).
