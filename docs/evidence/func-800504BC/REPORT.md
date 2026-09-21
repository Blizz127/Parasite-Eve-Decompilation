# func_800504BC — MATCHED (13 words, LINK_EXACT)

VRAM `0x800504BC`, file `0x40CBC`, span `0x34`. Carve: splits `[0x40B3C, asm]`;
the following `0x40CF0` slot is a gas `nop` pad.

```c
void func_800504BC(int a0) {
    func_8005E8A4(0, 1);
    func_80064C54(a0 + 0x33);
}
```

`a0` is pinned in `$s0` (callee-saved) across the first call, then `+0x33` is
formed in the `jal` delay slot. era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
