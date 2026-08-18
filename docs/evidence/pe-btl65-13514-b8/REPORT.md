# PE-BTL65 — type-0 persist==39 opcode 0xB8

After Watch `0x31` M0367I, type-1 sets `persist[0x4A]=0x27`
and hops back to M0005I. 125E0 still spawns type 1 then 6;
type-1 then `0x08`s types 3, 0, 5, and 2 (`persist<0x28`).

Type-0 persist==0x27 takes `0xAA`/`0x40`/`0x65`, the fade
and messages `0x3B`/`0x84`, then `+0x478` opcode `0xB8`.

## `func_80013514`

107 words `0x80013514..0x800136C0`, SHA-256 `573a82c6…4ee7`.
`D_800910A0[0xB8]`. `jal 0x80079FB4`. No TEXT `jal` sites.

First visit latches `*arg0/*arg1/*arg2` into task
`+0x14/+0x18/+0x1C` and sets task+8 bit `0x20`.
`dx/dz = (actor+0x28/+0x30 - latch) >> 16`. Both zero
returns 1 and leaves the bit set. Else the 0x4B angle
step: `desired = (0x1400 - ratan2(dZ,dX)) & 0xFFF`.
Unfinished facing rewinds `CE00` by `0x14` and returns 0.
Finished facing clears bit `0x20` and returns 1.

Live imms: `0x40D0000`, `0xFF670000`, `0xB4`.
