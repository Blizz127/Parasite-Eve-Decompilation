# func_8005DBAC — PARKED (address-materialization / schedule residual)

19 words, VRAM `0x8005DBAC`, file `0x4E3AC`. Clamp arg0 to [0,98], then return
`D_800A803C + (24*idx + (&D_800A803C - 0x14))` (a 24-byte-stride 100-entry
table whose base sits 0x14 bytes before the symbol; `D_800A803C` holds the
table's data pointer).

The clamp prologue is exact with `register int idx asm("$3")`, and the index
math `idx*24` (`sll/addu/sll`) is exact. Residual: retail materializes the
symbol address **once** into `$a0` and derives both uses from it —

```
addiu v0, a0, %lo(D_800A803C)   ; address value
...
addiu v1, a0, -0x14             ; base-0x14 (one lui)
lw    a0, 0(a0)                 ; reload the pointer word
addu  v0, v0, v1
jr    ra
addu  v0, a0, v0
```

cc1 always materializes the symbol **twice** (`lui $4` + `lw`, and a separate
`lui $4` + `addiu $4,-0x14`) or loads before computing the offset, so the load
and `addiu` swap. Mismatch ladder: plain `-O2 -G0` 18; pinned `idx asm("$3")`
9; `char D_800A803C[]` array form 7; operand-order sweep (all 6 permutations of
value/offset/index) 9-10; explicit `off`/`val` temps 8; pinning the pointee to
`$4` 8; a local `char *b = &D_800A803C` base 16-17 (cc1 splits the load/store
through `b`). Best 7 mismatches are pure instruction order (same ops). Same
class as the `func_80073A44` prologue-scheduling park.
