# func_800193D8 — MATCHED (14 words, LINK_EXACT)

VRAM `0x800193D8`, file `0x9BD8`, span `0x38`. The `[0x9BD8, asm]` span is
exactly the function; `func_80019410` starts at `0x9C10`.

```c
int func_800193D8(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_80065AD4(*(unsigned int *)v0, *(unsigned int *)v1, *(unsigned int *)a2);
    return 1;
}
```

All three arguments are **words** (`lw`) — the word-arg third of the
`func_80015AB8` / `func_80018B30` pointer-wrapper family. era `-O2 -G0`.
Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
