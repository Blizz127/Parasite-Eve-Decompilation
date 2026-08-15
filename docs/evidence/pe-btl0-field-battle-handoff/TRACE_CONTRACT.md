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
| `transition_state` | enum | `field` `mailbox_3` `mailbox_4` `m0005i_enter` `slots_ready` `mode6_request` `mode6_consumed` `wait_mode7` `post_return` |
| `battle_mode` | `D_8009D28C` raw | 0/3/4/5/6/7/8 |
| `formation_id` | `49` or `50` plus `1332/1333/1334` | write as `49;1332,1333,1334` |
| `player_state_hash` | SHA-256 of Aya actor bytes that battle actually touches | **do not invent a span**; until HP layout is proven, hash `D_8009D254` object header + pose `+0x28..+0x3A` only and mark `partial` |
| `persist_hash` | SHA-256 of `D_800A77F0` .. `+0x800` | PST0 bank; expect equality across `0x89` |
| `rng_state` | 14-word lagged-Fibonacci image used by `70D10` | dump only when `0x1A` runs |
| `battle_tick` | 0 while `field`; increment after mode-6 consume | independent of ATB |

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
