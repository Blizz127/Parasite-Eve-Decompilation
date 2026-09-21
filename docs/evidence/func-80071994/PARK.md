# func_80071964 / func_80071994 — PARKED (duplicate-nop / branch-retarget class)

12 words each, VRAM `0x80071964` / `0x80071994`, file `0x62164.s`. Conditional
offset getters: return `(a0 + 8) + 4` (resp. `+ 12`), plus
`*(unsigned int *)(a0 + 8)` when `*(unsigned int *)(a0 + 4) & 8` is set.

Retail shape (`beqz`/`j` overlap, 48 bytes):

```
lw    v0,4(a0)
nop
andi  v0,v0,0x8
beqz  v0,.L
nop
lw    v1,8(a0)
addiu v0,a0,8
j     .L2
addu  v0,v0,v1
.L:
addiu v0,a0,8
.L2:
jr    ra
addiu v0,v0,4
```

cc1 emits the same ops minus the two `nop`s (`beq $2,$0,$L2` with the
`addu $3,$4,8` in its delay slot), so `size = 40` vs the 48-byte span and the
branch offsets differ — 8 mismatches, invariant across `-O2/-O1`, `-G0/-G8`,
`-fno-delayed-branch`, a `p` local / `if`-`else` local (which adds a branch and
a pad) and a `w` local. Same duplicate-`nop`/retarget class as the
`func_8006F9F0` park: retail kept redundant `nop`s that GCC's fill removed.
