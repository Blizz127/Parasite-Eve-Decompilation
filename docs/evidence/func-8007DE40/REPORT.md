# func_8007DE40 — MATCHED (14 words, LINK_EXACT)

VRAM `0x8007DE40`, file `0x6E640`, span `0x38`. Carve: splits
`[0x6E5A8, asm]`; the span ends exactly at `0x6E678` where
`func_8007DE78` starts.

```c
void func_8007DE40(void) {
    func_80072714();
    func_8007E324();
    func_80073C74(0);
    func_80072724();
}
```

Four-call void wrapper (no `return` materialization). era `-O2 -G0`.
Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
