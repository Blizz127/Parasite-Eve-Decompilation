# func_80050308 — MATCHED (13 words, LINK_EXACT)

VRAM `0x80050308`, file `0x40B08`, span `0x34`. Carve: splits the former
`[0x40A80, asm]` span; `func_8005033C` starts exactly at `0x40B3C`.

```c
void func_80050308(int a0) {
    if (D_8009CF18) func_8005EB64(a0 + 0x7C);
    else            func_8005EB64(a0 + 0x7F);
}
```

The `$gp`-relative `D_8009CF18` (`lw 0x1A8($gp)`) forces the **`-O2 -G8`**
profile (registered under `era_o2_g8` in `disc1_build_profiles.json`).

**Lever:** the branch polarity is load-bearing. Writing the *nonzero* case
first (`if (D_8009CF18)`) reproduces retail's `beqz $v0` into the `+0x7C` arm;
the `== 0`-first form emits the opposite polarity (3 word mismatches).

Gate: `check_leaf.sh func_80050308 0x80050308 0x34 -O2 -G8` → `LINK_EXACT`,
deep span exact.
