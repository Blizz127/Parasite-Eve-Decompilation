# func_8005022C — MATCHED (13 words, LINK_EXACT)

VRAM `0x8005022C`, file `0x40A2C`, span `0x34`. The span begins exactly at the
function; `func_80050260` follows at `0x40A60`.

```c
void func_8005022C(unsigned char *a0) {
    D_8009CEF4 = (int)a0;
    D_8009CF20 = D_8009CF58;
    func_800638D8(a0, func_80050AD8);
}
```

**Lever — statement order is load-bearing across two `$gp` stores.**
`D_8009CEF4` is `0x184($gp)` and `D_8009CF20` is `0x1B0($gp)` (gp base
`0x8009CD70`). Writing the `a0` store *first* gives retail's
`sw $a0,0x184($gp)` / `lw $v0,0x1E8($gp)` / `sw $v0,0x1B0($gp)` sequence;
swapping the two stores (or making them `volatile`) emits them in the other
order and produces 2-12 mismatches. This is a plain permutation of independent
non-aliasing stores.

The leaf reads `$gp` slots, so it needs the **`-O2 -G8`** profile.

Gate: `check_leaf.sh func_8005022C 0x8005022C 0x34 -O2 -G8` -> `LINK_EXACT`,
deep span exact.
