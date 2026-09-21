# func_80080F64 — MATCHED (13 words, LINK_EXACT)

VRAM `0x80080F64`, file `0x71764`, span `0x34`. Carve: splits the former
`[0x7155C, asm]` span; `func_80080F98` starts exactly at `0x71798`.

```c
void func_80080F64(int a0) {
    if ((a0 & 0xFF) == 2) func_80081D74(func_80080F98, -1);
}
```

`func_80080F98` is passed as a **function address** (`lui`/`addiu` HI16/LO16
relocs) with `-1` in the delay slot. era `-O2 -G0`, default profile.
Gate: `check_leaf.sh func_80080F64 0x80080F64 0x34 -O2 -G0` → `LINK_EXACT`,
deep span exact.
