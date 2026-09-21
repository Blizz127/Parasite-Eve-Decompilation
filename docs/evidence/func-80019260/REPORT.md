# func_80019260 — MATCHED (14 words, LINK_EXACT)

VRAM `0x80019260`, file `0x9A60`, span `0x38`. The `[0x9A60, asm]` span is
exactly the function; `func_80019298` starts at `0x9A98`.

```c
int func_80019260(void) {
    func_8003E0D0(D_8009D2F0 + 0x1B4);
    *(unsigned int *)(D_8009D2F0 + 0x18C) = 0;
    return 1;
}
```

`D_8009D2F0` is loaded into `$a0` **before** the frame (`lui`/`lw` ahead of
`addiu $sp`), and the `+0x1B4` displacement stays on the `addiu` in the call
delay slot. The second use reloads the global. era `-O2 -G0`.
Gate: `check_leaf.sh` → `LINK_EXACT`, deep span exact.
