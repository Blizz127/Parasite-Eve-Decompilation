# `func_80083D9C` — parked compiler residual

Disposition: `PARKED-SWITCH-TAIL-BLOCK-LAYOUT`.

## Function hood and boundaries

The retail span is `[0x80083D9C,0x80083DF0)`, 0x54 bytes / 21 words. It
ends with `jr ra`/`nop` at `0x80083DE8/0x80083DEC`. Its exact-start callback
table references are the pointer stores at `0x80083D50`, proving it callable
despite no direct `jal` callers. The preceding function returns at
`0x80083D94/0x80083D98`; the following function begins at `0x80083DF0`.
Both boundaries are real.

## Retail behavior

The function reads byte `0x46` of its object. Mode 2 writes kind `0x44`,
stores `object+0x51` at offset `0x2C`, and copies the mode to offset `0x35`.
Mode 3 writes kind `0x4D`, stores `object+0x5D`, and writes tag `6`. Other
modes perform no stores.

## Bounded attempts

The first if/else phrasing emitted a useful but nonmatching 0x50-byte body.
The second switch phrasing recovered the desired `$v1` mode register and
constants, but cc1 reloaded byte `0x46` in the mode-2 case and emitted a
0x60-byte body with a different shared-tail layout. Retail's 21 words include
the branch-delay constants, both local-jump slots, and the common return.

No pins, inline assembly, or forged padding were used. The residual is
compiler switch block layout and reload scheduling; the leaf is not integrated
and the matching-C count remains 335.
