# func_80018B30 — MATCHED (14 words, LINK_EXACT)

VRAM `0x80018B30`, file `0x9330`, span `0x38`. The `[0x9330, asm]` span is
exactly the function; `func_80018B68` starts at `0x9368`.

```c
int func_80018B30(unsigned char *a0) {
    unsigned int v0 = *(unsigned int *)(a0 + 0);
    unsigned int v1 = *(unsigned int *)(a0 + 4);
    unsigned int a2 = *(unsigned int *)(a0 + 8);
    func_800679C4(*(short *)v0, *(short *)v1, *(short *)a2);
    return 1;
}
```

All three arguments are **halfwords** (`lh`), the short-arg twin of
`func_80015AB8`. era `-O2 -G0`. Gate: `check_leaf.sh` → `LINK_EXACT`, deep
span exact.
