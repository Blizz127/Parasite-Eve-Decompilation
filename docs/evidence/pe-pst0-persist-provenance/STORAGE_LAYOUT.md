# STORAGE_LAYOUT — retail persist banks

Evidence-only. Re-derived from USA Disc 1 `SLUS_006.62`
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and
`func_80034F10` / binder mode-2 / `func_8003F800`.

Command:

```text
python3 tools/research/pe_pst0_scan.py "$PE_DISC1_BIN"
```

Disc 1 SHA-256 `7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4`
(full raw MODE2/2352). EXE SHA-256
`5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b`.
PE.IMG SHA-1 `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`.

## Canonical persist[]

| Field | Value | Evidence |
|---|---|---|
| symbol | `D_800A77F0` | `lui %hi` / `addiu %lo` at `0x800171CC` |
| base | `0x800A77F0` | EXE image; splat label |
| element width | 32-bit word | binder `sll 2`; `lw`/`sw` |
| count | `0x200` = 512 | `func_80034F10` `sltiu $a1, 0x200` |
| size | `0x800` = 2048 bytes | save/load `addiu $t0, base, 0x800` |
| signedness | stored as word | compares use `slt` (signed); `==`/`!=` xor |
| EXE image contents | 512 zero words | BSS-shaped zeros in the EXE |
| alias | field VM binder **mode 2** | `0x800171BC` |

Index arithmetic: the script operand is a 32-bit slot. The binder does
`slot << 2` and adds `D_800A77F0`. There is no retail bounds check in
the mode-2 case.

## Do not merge these banks

Python/clean-runtime names that look like “persist” or “local” map to
four distinct retail arrays.

| Binder mode | Base | Width | Count proven | Zeroed by `34F10` | In `func_8003F800` 0x800 copy | Role |
|---:|---|---|---|---|---|---|
| 0 | command immediate | word | n/a | n/a | no | immediates |
| 1 | `*D_8009D2F0 + 0xAC + slot*4` | word | actor-local | actor pool zero | no | task/actor locals |
| 2 | `D_800A77F0 + slot*4` | word | 512 | yes | **yes** | global persist |
| 3 | `D_8009DF70 + slot*4` | word | UNKNOWN | not in 34F10 persist loop | no | condition/ALU dest |
| 4 | `D_800B6A80 + slot*4` | word | 64 (`sltiu 0x40`) | yes (separate loop) | no | scratch |

Mode ≥ 5: `sltiu $v1, 5` fails; the binder stores nothing for that arg
(`0x80017204`).

`func_80034FC4` (field actor rebuild) walks `D_800BEA90` 13 × `0x280`
slots and does **not** touch `D_800A77F0`.

## Initialization / reset / copy

| Function | VA | Effect on persist |
|---|---|---|
| `func_80034F10` | `0x80034F10` | `sw $zero` 512 words; also zeros `D_800B6A80` (64) and 14 × `0x280` at `D_800BEA90`; clears several GP words; `D_800B0CD8 &= ~0x3000` |
| `func_8003E680` | `0x8003E680` | boot subsystem dispatcher; `jal func_80034F10` — new-game / boot reset |
| `func_80034FC4` | `0x80034FC4` | field load actor rebuild — **no persist write** |
| `func_8003F800` | `0x8003F800` | memcpy persist → `*D_800A0ED0` for `0x800` bytes (save) |
| `func_8003FBD8` | `0x8003FBD8` | memcpy `*D_800A0ED0` → persist for `0x800` bytes (load) |

No second persist base was found. EXE `lui 0x800A` / `addiu 0x77F0`
sites are exactly five: binder, zero, save, load, `func_80053128`.

## Survival (only what code establishes)

| Boundary | Persist bank | Evidence |
|---|---|---|
| room / field package load | survives | `func_80034FC4` does not store it |
| new game / boot | cleared | `func_8003E680` → `func_80034F10` |
| save | whole `0x800` serialized | `func_8003F800` |
| load | whole `0x800` restored | `func_8003FBD8` |
| battle | no clear found | no extra EXE store site; **not** a positive battle-keep proof |
| day boundary | UNKNOWN | no day-reset writer identified |

Scratch (`D_800B6A80`) and actor locals do **not** ride the persist
memcpy. They must not be treated as save-backed story state.
