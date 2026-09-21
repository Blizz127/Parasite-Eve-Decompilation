# func_8007DFE0 — MATCHED (12 words, LINK_EXACT)

VRAM `0x8007DFE0`, file `0x6E7E0`, span `0x30`. Carve: splits `[0x6E6C0, asm]`;
the next real function starts at `0x6E814` (a `sw $ra` prologue), so the 4-byte
`0x6E810` slot is gas alignment.

```c
int func_8007DFE0(void) {
    func_8007E1B4();
    func_80073C74(0);
    func_8007E204();
    return 1;
}
```

Three-call sequenced init; `return 1` lands in the `jr` delay slot. era
`-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
