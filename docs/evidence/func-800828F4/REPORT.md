# func_800828F4 — MATCHED (14 words, LINK_EXACT)

VRAM `0x800828F4`, file `0x730F4`, span `0x38`. Carve: splits `[0x72D74, asm]`;
`func_8008292C` starts at `0x7312C`.

```c
void func_800828F4(int a0, int a1) {
    int r = D_8009B738();
    func_80083BB8(r, a1);
}
```

The indirect call is a **zero-argument** function pointer (retail clears no
argument registers before `jalr`). **Lever — maspsx patch 3:** the epilogue
`addiu $sp,$sp,0x18` schedules into the `jr` delay slot; profile
`era_o2_g0_fill_epilogue_delay_slot`. era `-O2 -G0`. Gate: `check_leaf.sh`
(needs `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`) → `LINK_EXACT`, deep span exact at
775 c.
