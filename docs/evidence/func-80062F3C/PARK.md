# func_80062F3C — PARKED (prologue/epilogue scheduling residual)

24 words, VRAM `0x80062F3C`, file `0x5373C`, frame `0x18`. Find-then-forward:
walks `D_8009D154` for the first `type==1 && key24==key` node, then
`func_8006269C(node)`.

Body semantics reproduce exactly, but the prologue/epilogue scheduling is a
residual. Retail:

```
addiu sp,sp,-24
move  v1,a0          ; <- arg copied to $v1 BEFORE the frame store
lw    a0,996(gp)
li    a1,1
beqz  a0,...         ; <- $ra store scheduled into the guard delay slot
sw    ra,16(sp)
...
lw    ra,16(sp)
addiu sp,sp,24
jr    ra
nop
```

cc1 emits `addiu sp` / `sw ra,16(sp)` first and copies the arg into `$v1`
after. Mismatch ladder: `-O2 -G8` 8, `-O1 -G8` 12, `-O1 -G0` 8, `-O2 -G0` 12,
`-fno-delayed-branch` 8 (and adds a `nop`), `-fschedule-insns`/`-sched2` 8,
`-O3` 8, `-fomit-frame-pointer` 8. A split-guard rewrite (two sequential
`while` loops) is worse (15 mismatches, 128-byte body). The residual is a
prologue ordering choice, not a levers problem — same class as
`func_800701B4`.

Levers recorded for the (correct) body shape: `node->type == 1` hoists the
literal to `$a1`; the `break` form keeps the forward branch `beq $v0,$v1`.
