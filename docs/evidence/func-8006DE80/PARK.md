# `func_8006DE80` — PARKED (argument-forwarding shim; alloca/schedule residual)

Outcome: **PARKED**. No C committed to `src/` (an unmatched draft would violate
the repo rule that `src/` holds only verified leaves). The matched twin
`func_8006DCE4` is identical in shape with a different state global.

## Function hood and span

- File span `[0x5E680,0x5E6D4)` = 21 words. VRAM `[0x8006DE80,0x8006DED4)`.
- Fan-in 15 on the boot→day-2 path (callers include `func_8001D340`,
  `func_8001F814`, `func_80022210`, `func_800236E8`).
- Forwarding shim to `func_8006DED4(state, a0, a1, a2, a3, a4, a5)` with
  `state = D_800B0E08` (the twin `func_8006DCE4` uses `D_800B0E64`).

## Retail shape (asm/disc1/5E418.s)

```text
8006de80  addiu sp,sp,-0x20
8006de84  addu  v1,a1,zero
8006de88  sll   a3,a3,16
8006de8c  lw    v0,0x30(sp)      ; stack arg (word load!)
8006de90  sra   a3,a3,16        ; (short)a3
8006de94  sw    a3,0x10(sp)
8006de98  sll   a3,a2,16
8006de9c  addu  a1,a0,zero
8006dea0  addu  a2,v1,zero
8006dea4  lui   a0,%hi(D_800B0E08)
8006dea8  lw    a0,%lo(D_800B0E08)(a0)
8006deac  sra   a3,a3,16        ; (short)a2
8006deb0  sw    ra,0x18(sp)
8006deb4  sll   v0,v0,16
8006deb8  sra   v0,v0,16        ; (short)a4
8006debc  jal   func_8006DED4
8006dec0  sw    v0,0x14(sp)     ; (delay)
8006dec4  lw    ra,0x18(sp)
8006dec8  addiu sp,sp,0x20
8006decc  jr    ra
8006ded0  (nop)
```

The load of the outgoing stack argument at `0x30(sp)` is a **full word** `lw`
(not `lh`/`lhu`), so the incoming parameter is `int`-typed, and the
sign-extension is an explicit `sll`/`sra` pair for each of `a2`, `a3`, `a4`.
`func_8006DED4`'s parameters are declared `unsigned short`, so a
`(short)`-typed callee prototype makes cc1 drop the `lw` (uses `lh`/`lhu`) —
the residual tracks the *callee's* int-promotion, not this wrapper's.

## Residuals (best object diff)

With the callee declared `(…, short, short)` and the wrapper body
`func_8006DED4(D_800B0E08, a0, a1, (short)a2, (short)a3, (short)a4)`, the
24-word target still shows **16 mismatched words** (object size `0x54` = ROM).

Two coupled residual classes, both compiler-internal:

1. **Argument-promotion shape.** cc1 computes the sign-extension of `a4` *after*
   the `jal` return value is moved into `$v0` (`lw v0 / sll / sra / sw` ordered
   as `sll; sra; sw ` in retail but `sll; sw` before the `jal` in cc1's output),
   and shortens the `$a3` copy through `$v1` in a different order than retail.
2. **Schedule.** Even matching instruction *set*, cc1 places the
   `lui/lw D_800B0E08` and the two `(short)` conversions in a different linear
   order (`-O2 -G0`, `-O1 -G0`, `-O1 -G0 -fschedule-insns2` all byte-identical
   residuals; `-G0` word-load access is already correct).

Tried shapes (all `@ 0x8006DE80, 0x54`): `(short)` casts vs implicit
conversion; `unsigned short`/`short`/`int`/unknown-prototype callee
declarations; a local `int t = a4` temp before the call; `a4` declared
`unsigned int`/`short`. Best is 16 mismatched words; no shape reached a
relocation-only diff.

Also parked with the same shape: `func_8006DCE4` (twin, `D_800B0E64`).

## Reachability / next lever

The residual looks like a GCC-2.7.2 argument-conversion scheduling artifact.
A future lever would be a maspsx patch that recognises the outgoing
integer-argument sign-extension prologue of a tail call and reorders the
`sll/sra/sw` triple to the ASPSX order — but that is speculative and not
implemented. Left parked per the mandate.
