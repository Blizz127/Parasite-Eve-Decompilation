# func_8003F3C4 — LINK_EXACT

Field-tick / main-loop body. Sole disc-1 TEXT caller is `func_8001220C`
(`jal` at `0x800123D8`). VRAM `0x8003F3C4`, file `0x2FBC4`, size `0x394`
(229 words), one terminal `jr $ra`, next glabel `func_8003F758`.

## Command

```
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009CDD8,D_8009D1F4,D_8009D238,D_8009CDDC,D_8009D250,D_8009D26C,D_800B0CEA,D_800B0CD8,D_800BCFE8 \
  tools/analysis/check_leaf.sh func_8003F3C4 0x8003F3C4 0x394 -O2 -G8
```

Result: `LINK_EXACT`, 0 word mismatches; compiled `.text` equals declared
span; no interior `jr $ra`. Object file carries 12 bytes of gas alignment
pad past the body (`nonzero_pad=0` over the declared size).

Profile: `era_o2_g8_force_3f3c4_absolute`.

## Levers

1. `-O2 -G8` keeps `D_8009D1C4` / `D_8009D280` / `D_8009D1A0` / `D_8009CDA4`
   gp-relative (`0x454` / `0x510` / `0x430` / `0x34($gp)`).
2. `MASPSX_FORCE_ABSOLUTE_SYMBOLS` for the in-range words retail materializes
   with `lui`/`lw` (`D_8009CDD8`, `D_8009D1F4`, `D_8009D238`, `D_8009CDDC`,
   `D_8009D250`, `D_8009D26C`) plus the far byte `D_800B0CEA` (otherwise
   `-G8` emits a truncated `R_MIPS_GPREL16`).
3. `s2 = &D_800B0CEA` then an empty `asm volatile` barrier so
   `s0 = s2 - 0x12` stays `addiu $s0,$s2,-0x12` instead of reassociating to
   `la $s0, D_800B0CD8` (same class as `func_8005DB44` / `func_800811E4`).
4. First flags load is `*(unsigned int *)(s2 - 0x12)` (`lw -0x12($s2)`);
   later loads go through `s0`.
5. Mask `~0x30` pinned `asm("$3")` so it lands in `$v1` ahead of the flags
   word in `$v0`.
6. Dest-change tail pinned `$2`/`$3`/`$4` so `D_8009D1A0` / `D_800B0CD8` /
   `~0x3800` match retail's `$v0`/`$v1`/`$a0` homes; scalar `D_800B0CD8`
   (not an array) keeps the `%lo` form instead of `la $s0`.
