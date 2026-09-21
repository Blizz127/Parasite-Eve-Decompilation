# func_80085EB4 — 0x5C bytes — PARKED

Retail VRAM `0x80085EB4` (file offset `0x766B4`, size `0x5C`, 23 words), in
`asm/disc1/765E8.s`. Fan-in 5. Not registered as a `c` span; stays `asm`.

## Retail semantics (proven)

Range-gated `func_8007DB24(-1)` result, shifted by a second global:

```c
extern unsigned short D_8009B414;
extern int D_8009B424;
extern int func_8007DB24(int a0);

unsigned int func_80085EB4(int a0) {
    if ((unsigned int)(a0 - 0x1010) > 0x7EFE8)
        return 0;
    D_8009B414 = func_8007DB24(-1);
    return (unsigned int)D_8009B414 << D_8009B424;
}
```

The guard is `0x7EFE8 < a0 - 0x1010` (retail materializes `0x7EFE8` then
`sltu`), and the result is re-read from the global, not reused from `$v0`.

## Residual mechanism — `sllv` shift-amount register + `andi` fold

- Retail does `sllv $v0,$a0,$a1`: the shift **amount** is in `$a1` (moved up at
  entry by `addu $a1,$a0,$zero`), while the value being shifted is in `$a0`.
  era cc1 puts the value in `$v0` and the amount in `$v1`
  (`sllv $v0,$v1,$v0`) — a register-home skew, and the prompt `addu $a1,$a0`
  is absent.
- Retail re-reads `D_8009B424` through one `lui`/`lw` pair but cc1 rematerializes
  the symbol base twice (`lui $v1`/`lui $v0`), a 4-byte size difference.
- The `lhu` re-read of `D_8009B414` appears in both, but retail keeps it in
  `$v1` and cc1 in `$v0`, so the `andi $2,$2,0xffff` fold differs.

Five phrasings (direct, `volatile` global, `unsigned`/`int` variants, explicit
re-read local, `(a0 - 0x1010) > 0x7EFE8` vs `<` inversion) are invariant on the
default rung. Same class as the `func_80070D6C` register-home residuals.

Next-possible-unblocker: a frontend that assigns the `sllv` amount the callee
`$a1` home and CSEs the two symbol bases.
