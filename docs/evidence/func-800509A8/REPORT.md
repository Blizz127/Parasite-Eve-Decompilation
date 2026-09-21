# func-800509A8 — MATCHED (14 words, LINK_EXACT)

VRAM `0x800509A8`, file `0x411A8`, span `0x38`. Carve: splits `[0x40F48, asm]`.

```c
void func_800509A8(int a0) {
    func_8005EB58(0);
    func_80064C54(D_8009CF14 + a0);
}
```

The `lw 0x1A4($gp)` slot is **`D_8009CF14`** (gp base `0x8009CD70` + `0x1A4`),
so the leaf needs the **`-O2 -G8`** profile; the `$s0` pin holds `a0` across
the first call. era `-O2 -G8`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep span
exact.
