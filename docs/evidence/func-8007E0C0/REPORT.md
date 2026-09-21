# func_8007E0C0 — MATCHED (14 words, LINK_EXACT)

VRAM `0x8007E0C0`, file `0x6E8C0`, span `0x38`. Carve: splits `[0x6E6C0, asm]`.

```c
int func_8007E0C0(void) {
    func_80072714();
    func_8007E1F4(1, D_800A34B0);
    func_80072724();
    return 1;
}
```

`D_800A34B0` is passed as a **data address** (HI16/LO16), the `1` in the `jal`
delay slot. era `-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span
exact.
