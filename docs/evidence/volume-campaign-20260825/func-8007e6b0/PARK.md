# `func_8007E6B0` — parked compiler residual

Disposition: `PARKED-SYMBOLIC-ADDRESS-REGISTER-COLORING`.

## Function hood

The retail span is `[0x8007E6B0,0x8007E704)`, 0x54 bytes / 21 words. It
ends with `jr ra` and a `nop` at `0x8007E6FC/0x8007E700`. Four exact-start
callers are present at `0x8007EF18`, `0x8007F004`, `0x8007F21C`, and
`0x8007F314`. The preceding function returns at `0x8007E6A8/0x8007E6AC`;
the following function begins at `0x8007E704`. Both boundaries are real.

## Retail behavior

The function reads `D_800A3608[0]`. If it is at least 8 it returns null.
Otherwise it adds `D_800A3608[-2]`, wraps the selected index by subtracting
8 when the sum reaches 8, and returns the corresponding 24-byte record
address from the table base at `D_800A3608 - 3`.

Retail's decisive register shape is:

```text
v1 = &D_800A3608
a0 = v1[0]
...
lw v0,-8(v1)
...
addu v1,v1,-0xC
addu v0,v0,v1
```

The address base survives as `$v1` while the loaded index is `$a0`.

## Bounded attempts

Attempt 1 used a local pointer and produced the right arithmetic but selected
`$a0` for the base, added a separate symbolic materialization for the `-8`
load, and emitted 0x60 bytes. Attempt 2 used a struct overlay anchored at
`D_800A3600.index`; it produced the same 0x60-byte `$a0`-base shape. The
residual is register allocation/address lifetime, not semantics or a missing
source operation. Both attempts were under era `-O2 -G0`; no pins, inline
assembly, or forged padding were used.

## Result

Retail is 21 words; both candidates are 24 words before any trim and therefore
cannot be integrated. The leaf remains excluded from YAML and the matching-C
count remains 335. A future unblocker would need a proven source-level way to
make cc1 retain the symbolic base in `$v1` while using `$a0` for the loaded
index, or a toolchain change with full regression evidence.
