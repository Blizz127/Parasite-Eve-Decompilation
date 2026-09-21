# func_8005E4E4 / func_8005E518 — MATCHED (13 words each, LINK_EXACT)

VRAM `0x8005E4E4` / `0x8005E518`, files `0x4ECE4` / `0x4ED18`, span `0x34`
each. Carve: splits the former `[0x4E92C, asm]` span into prefix `0x3B8`,
C `0x34`, C `0x34`, resume `0x1C` (`func_8005E54C` follows at `0x4ED4C`).

```c
extern int D_8009D0E8;
extern unsigned int func_8005E038(void);

int func_8005E4E4(void) {
    int r = 0;
    if (D_8009D0E8) {
        unsigned int v = func_8005E038() & 0x20;
        r = v != 0;
    }
    return r;
}

int func_8005E518(void) {
    int r = 0;
    if (D_8009D0E8) {
        unsigned int v = func_8005E038() & 0x5000;
        r = v != 0;
    }
    return r;
}
```

**Semantics.** Twin gp-guarded status-bit queries over the `D_8009D0E8`
(`gp+0x378`) enable flag. Each returns `1` when the corresponding
`func_8005E038()` response bit is set (`0x20` = one bit, `0x5000` = two bits
or'ed together) and `0` otherwise, including when the subsystem is inactive.
`func_8005E038` is the shared input-status reader.

**Lever — keep the masked value in a local, then `!= 0`.**
`r = (func_8005E038() & 0x20) != 0;` folds the mask into the compare
(`sltu $2,$0,$3` + `ne $3,$2,$0`). Writing the mask as an `unsigned int v`
local first (`unsigned int v = func_8005E038() & 0x20; r = v != 0;`)
reproduces retail's `andi $v0,$v0,0x20` then `sltu $v1,$zero,$v0`.
Mismatch: 2 without the local, 0 with it. `r = (v > 0)` is codegen-identical.

**Build profile.** `era_o2_g8` (the guard load is gp-relative, offset 0x378).

**Gate.**

```
tools/analysis/check_leaf.sh func_8005E4E4 0x8005E4E4 0x34 -O2 -G8 → OK
tools/analysis/check_leaf.sh func_8005E518 0x8005E518 0x34 -O2 -G8 → OK
  → LINK_EXACT (both), word mismatches=0
  → disc1_preflight: PASS (deep, 789 c / 347 asm / 2 rodata)
```
