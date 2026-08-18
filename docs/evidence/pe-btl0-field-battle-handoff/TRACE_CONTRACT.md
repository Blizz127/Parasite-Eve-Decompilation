# TRACE_CONTRACT — field/battle boundary

Neutral columns for a future Python / native / UE oracle. One row per
field tick through the hop, then per battle tick until mode==7.

```text
field_tick
field_scene
field_script_pc
encounter_id
transition_state
battle_mode
formation_id
player_state_hash
persist_hash
rng_state
battle_tick
```

## Encoding

| Column | Source | Notes |
|---|---|---|
| `field_tick` | existing field frame counter | continues across the hop |
| `field_scene` | packed name | `m0004i` then `m0005i` |
| `field_script_pc` | current module offset | e.g. `mod4+0x1040`, `mod6+0x350C` |
| `encounter_id` | first request id | `m0005i_mod6_350C` until a better retail id exists |
| `transition_state` | enum | `field` `mailbox_3` `mailbox_4` `m0005i_enter` `rng_selected` `slots_ready` `mode6_request` `mode6_consumed` `wait_mode7` `encounter_55` `hp_copied` `first_command` `command_bound` `overlay_wait` `overlay_cleared` `input_held` `actors_captured` `attack_available` `hp_mutated` `encounter_complete` `post_return` |

`first_command` is legal only after the ROM-verified post-`293F4` tail:
`0x800299AC` loads record `+0x12` (the preceding HP cut stored byte `4`),
`0x800299B0` calls `func_8001A680`, and `0x8001A6D0` stores that byte at
actor `+0x0E`. It names the first actor animation/clip command issue, not a
battle-menu selection, ATB action, damage command, or invented command word.
`command_bound` is legal only after that same `1A680` prefix stores
`D_800B0E98[type*192 + command*4]` at actor `+0x1B0`. The table slot is
filled by Writer B (`sw` at `0x8006C158`: `overlay+0x1C0+idB*4`) from a
clip-directory package; CE2=14 maps to PE.IMG `[396,428)` which contains
type-0 `idB=4`. `func_80029810` does not jal the writer. Not overlay
completion, ATB, mode 7, or `0x55` return.
`overlay_wait` is legal only while `0x55` is in state `0x3B` and
`D_800B0CD8+0xE & 3 != 0`. That gate is `lbu +0xE; andi 3` at
`0x80014630`; it does not jal `func_8006914C`. State `0x39` is the
`jal 6914C(1)` site (`0x800145DC`) and runs *before* `0x3A`/`29810`.
State `0x3A` oris `D_8009D1A0` bit 1, then jals `29810`. The next
field tick's `func_8003F074` (first jal of `3F3C4`) jals `6C4C4(CE4)`
then polls `6C5BC` until `v0!=1`. `35558` also jals `6C5BC` once after
the actor walk. Those are the live callers; `144FC`/`29810` do not jal
`6C5BC`. `6914C` never stores `+0xE`. Native emits `overlay_wait` after
`command_bound` when that 3F074 tail sets bits 0-1. `overlay_cleared`
is legal only after the EE=13 epilogue at `0x8006CC2C` (`andi 0xFC`
immediately after `jal 3D834`) makes `(+0xE & 3)==0`. It does not
emit `post_return`, stub `6914C`, or store mode 7.
`input_held` is legal only after `func_8003EB04` rebuilds `D_8009D26C`
from `lhu 0x800BE9A2` (active-low) through `A76F0`. Planting `D26C`
directly is not this state. `actors_captured` is legal only after a
type-0 `35C84` tick that records pose `+0x28/+0x30`, `D26C`, and
`D_8009D28C`. `attack_available` is legal only when a retail gate
opens. The BTL72 playable loop emits it after type-3 `0x85`
(`18EB4`) once `1CAB0` returns 1 on rect1 from the live 0x0B
pose plus 409 Right ticks (`BE9A2=0xFFDF`). It is not ATB,
menu Attack, rec=4, or HP damage. Live type-3 ops after that
`0x85` are `0x9C` / `0x0A` persist[1]=5 / `0x31` `0xA8000248`
(M0004I door). That hop is not `field_return`. Do not emit
`attack_available` from a planted `D26C` or a planted pose
inside the rect. `hp_mutated` is
legal only when record `+0x0C` changes from a retail
damage writer (`1F704` inside `1F4D4`, jal from `1D340`),
not from the `293F4` copy. First retail subtract is 40→39;
the next live store is 39→34. After a nonzero remainder,
`1F814` issues `1A680` on `D254` (hit-react). Death is the
`1D340` `1F080` HP<=0 path (mode 3, `4D4=0`, `1A680(19)`,
`1F4B0` zero). Next tick `2A7F8` mode 3 takes `2B29C`
on the zero fixture (`2AA98` if bit `0x800` /
`53E6C(18)`). `2B29C` case 5 stores mode=-1 and `6A25C` dest
`0xA9400048` (player-death / title, not victory).
Same tick `3F3C4` keeps dest-change after the
`0x100` draw skip; `1220C` then restarts the outer
boot loop. Title dispatch / field return are not
this cut. Do not invent pad / hit / rec=4 / `4D4` /
mode 7 / HP.
| `battle_mode` | `D_8009D28C` raw | 0/3/4/5/6/7/8 |
| `formation_id` | `49` or `50` plus `1332/1333/1334` | write as `49;1332,1333,1334` |
| `player_state_hash` | SHA-256 of Aya actor bytes that battle actually touches | BTL2: SHA-256 of record `+0x0C/+0x0E/+0x1C` (HP triple, `func_800293F4_hp_cut`). BTL1 rows stay `partial` |
| `persist_hash` | SHA-256 of `D_800A77F0` .. `+0x800` | PST0 bank; expect equality across `0x89` |
| `rng_state` | 19-word image `0x80070E04..0x80070E4C`: two indices + 17-word lagged-Fibonacci table | dump immediately before `0x1A` runs |
| `battle_tick` | 0 while `field`; increment after mode-6 consume | independent of ATB |

NYPD / Eve-intro arm is `encounter_55` at module 6 `+0x4140`
(`0x55(2)`), with `battle_mode` still 0. `0x89` is the next
opcode (`+0x414C`). Do not emit `mode6_consumed` on that arm
unless `0x89` actually ran.

## Hash policy

- Same as SYS0 UE parity: SHA-256 of the CSV bytes.
- `persist_hash` must not change on the `0x89` row unless a field
  `0x0A` also ran that tick.
- Representation-only diffs: host pointer width in debug columns.
  Mode and formation cells are exact.

## Minimum rows for BTL1

1. m0004i walk into volume `+0x0F78` or `+0x10E4`
2. mailbox 3 or 4
3. `0x31 m0005i`
4. m0005i `0x1A` (record rng_state + 49/50)
5. `0x6F` / `0x70` / `0x5A` 1332/1333/1334
6. `0x89` (`battle_mode=6`)
7. first consume (`battle_mode=0`, `transition_state=mode6_consumed`)
8. stop — do **not** require ATB rows in BTL1
