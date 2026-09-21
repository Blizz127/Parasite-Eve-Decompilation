# func_800501C8 — MATCHED (14 words, LINK_EXACT)

VRAM `0x800501C8`, file `0x409C8`, span `0x38`. Carve: splits `[0x408A8, asm]`.

```c
void func_800501C8(unsigned char *a0) {
    D_8009CEF4 = (int)a0;
    func_800638D8(a0, func_8005100C);
    func_8005EB58(1);
    func_8005EB64(0x68);
}
```

The `sw $a0, 0x184($gp)` slot is **`D_8009CEF4`** (gp base `0x8009CD70` +
`0x184`), so the leaf needs the **`-O2 -G8`** profile. era `-O2 -G8`.
Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
