# func_8004F7D8 — MATCHED (12 words, LINK_EXACT)

VRAM `0x8004F7D8`, file `0x3FFD8`, span `0x30`. Carve: splits `[0x3FC64, asm]`;
`func_8004F808` starts exactly at `0x40008`.

```c
void func_8004F7D8(void) {
    func_8005E8A4(6, 6);
    func_80053648(D_8009CF58);
}
```

`D_8009CF58` is a **`$gp` slot** (`lw 0x1E8($gp)`, gp base `0x8009CD70` + `0x1E8`),
so the leaf needs the **`-O2 -G8`** profile. era `-O2 -G8`. Gate:
`check_leaf.sh` → `LINK_EXACT`, deep span exact.
