# func_8005FA3C / func_8005FB74 / func_8005FCAC / func_8005FDF0 / func_8005FF28 — PARKED

`0x138`-byte signed power-of-ten digit printers (five instances varying the
initial `digits` constant 2/3/4/5/6). `func_8005FA3C(v)`:

- `digits = K`; if `v < 0`: `v = -v`, `func_8005EB64(0x52)`, `digits = 1`,
  `D_8009D124 += 5`, `D_8009D128 = D_8009D128`.
- loop `i = 1 .. digits-1`: `p *= 10`.
- loop `i = 0 .. digits-1`: `q = v / p`; if `i < digits-1 && q == 0` then
  `q = -1`; `func_8005F874(q)`; `p /= 10`.

Best C attempt: `29-33` word mismatches (`-O2 -G8`, 304 bytes vs the 312-byte
span; `-O2 -G0` is worse). The residual is the **inline ASPSX div-by-zero
guard**: retail emits the full `bnez $s1` / `break 7` / `li $at,-1` /
`bne $s1,$at` / `lui $at,0x8000` / `bne $s3,$at` / `break 6` sequence around
`div`, and the `mflo` is scheduled after the guard's tail (3 words past the
`div`). maspsx's `--expand-div` produced only 256 bytes (a different, shorter
guard form), and the no-expand 304-byte form is still 2 words short with the
`mult`-based `p /= 10` schedule in the wrong place. The same residual applies
to all five instances.
