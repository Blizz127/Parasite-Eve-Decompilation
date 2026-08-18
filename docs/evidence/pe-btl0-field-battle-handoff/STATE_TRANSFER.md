# STATE_TRANSFER — field→battle

PST0: persist is `D_800A77F0`, 512 × 32-bit words.

## persist[]

Re-ran the PST0 lui/addiu census on this EXE. Still exactly five
sites: binder, new-game zero, save, load, `func_80053128`.

A bounded scan of `0x80029800–0x80031000` (battle cluster) finds
**no** additional `D_800A77F0` materialization.

```text
persist_bank_read_by_battle   = no_direct_exe_site
persist_bank_written_by_battle = no_direct_exe_site
battle_survival_status        = NO_CLEAR_FOUND   (same as PST0)
```

This is **not** a proof that battle never observes persist. It is a
proof that battle does not use the persist **base**. Field scripts on
m0005i still read/write persist around the fight (`0x0A`, `0x12`,
`0x50`, `0x54`, `0x64`, `0x4A`). Those are field VM accesses
(binder mode 2), not the battle tick.

`func_80053128` still reads other index ranges as item-like
`0x100–0x17F` values. Day-1 slots 0 / 1 / 0x4A are outside those
ranges (PST0).

## Aya / player

| Item | Evidence | Transfer |
|---|---|---|
| Actor object | `D_8009D254` loaded at battle init `0x8002996C` and by `0x5A` (`func_8002FF78` first load) | **referenced**, not copied into a new struct by `0x89` |
| Position `+0x28/+0x2C/+0x30` | no store in `0x89` | remains field actor |
| Map / token | no `0x31` on request | stay on m0005i |
| Control inhibit | `0x40` after the mode-7 poll | post-return, not pre-entry |

HP / MP / PE / equipment / level words inside the actor or the 216-byte
slot body are **not** mapped this rung. `0x5A` is the tagged writer
into that object (`func_8002FF78` switches on the first argument).

## Slot table

```text
D_800A5D58     7 records × 220 bytes
word0          inUse
+4             body[216]   (D_800A5D5C)
```

Matching leaves `func_8002F9CC` / `func_8002F970` already prove the
shape. `0x6F` (`func_8002F7D8`) walks the same stride, claims the
first free slot (`inUse=1`), and copies a 208-byte default from
`0x800109B0`.

`0x70` / `0xB7` write formation bytes into the current actor
(`D_8009D2F0`) at `index*16+0x1C` and `index*4+0x7C`.

## Temporary flags

| Address | Writer | Note |
|---|---|---|
| `D_8009D28C` | `0x89` / cluster | request / consume |
| `gp+0x10C` | `0x80029A04` | set when 6 is consumed |
| `D_8009D290` | init `sw $zero` | sibling of mode word |
| `D_8009D2E8` bit 2 | `0xAD` | `&= ~4` on m0005i after some fights |

## Party

No second party actor is created on the first m0005i setup. Slot
count 7 is a table capacity, not a proven party size.

## Seed

See `RNG.md`. Battle cluster does not jal the retail RNG trio.
The only pre-entry roll is script `0x1A`.
