# PE battle-data precovery

Authority: Disc 1 EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Disc 1 Form1 user data via `local/pe_disc1.path` / `PE_DISC1_BIN`.
PE.IMG LBA base `1013`. No matching `src/` C. No UE5 / gameplay edits.

This pack is a retail data contract for the live NYPD / m0005i encounter
and the immediate `0x31` dest `M0367I`. Identities are table/consumer
paths only. Do not name resources from appearance.

Verify:

```text
python3 pc_port/tools/pe_battle_data_precovery_oracle.py
```

`--write` regenerates the CSVs from EXE + PE.IMG.

## Live actor set

`125E0` desc at m0005i chunk2 `+0x25014` is `count=2`, type `1` then
type `6`. Type 6 is `D_8009D20C` head. Type 1 opcode `0x08` later
constructs types `3`, `0`, `5` in that order. Type 0 publishes
`D_8009D254`. Types `2` and `4` exist in the 12574 list of 7 and are
indexed here, but they are not on the 125E0 desc.

| type | first VM word | first op | spawn | +0x1AC source |
|---:|---|---|---|---|
| 0 | `0x0000609B` | `0x9B` | type1 `0x08` #2 | `6C118` CE2=14 `+0x8` |
| 1 | `0x0000C0EA` | `0xEA` | 125E0 desc[0] | empty on this path |
| 2 | `0x0000609B` | `0x9B` | listed only | `6B804` idB=2 |
| 3 | `0x00002002` | `0x02` | type1 `0x08` #1 | empty on this path |
| 4 | `0x00002002` | `0x02` | listed only | empty on this path |
| 5 | `0x00004014` | `0x14` | type1 `0x08` #3 | `6B804` idB=5 |
| 6 | `0x000080CE` | `0xCE` | 125E0 desc[1] | empty on this path |

Update vtable: type 0 `func_80035C84`; types 1–9 `func_80035E04`.

## Resource objects (`+0x1AC` / `+0x1B0` / `+0x1B4`)

Publication order on dest enter:

1. `func_8006B35C` (3F074 first jal, 103w, SHA-256
   `1106cb2a94fa877af5a067e36a3c24d08f186694b112c2139d2869f7cf341f24`)
   zeros `D_800B0E70[0..9]`, the ten `D_800B0E98` rows, and
   overlay `+0x944..+0x958`.
2. `func_8006B4F8` hdr+0x0C @ `0x8006B804` writes `D_800B0E70[idB]`.
   Live m0005i has **two** records: idB `2` and `5` only.
3. hdr+0x10 Writer A @ `0x8006B84C` writes
   `D_800B0E98[type*192+cmd*4]` from 21 room records.
4. hdr+0x14 / `12574` → `D_800B161C`; hdr+0x18 → `D_800B1620`
   (`0x8006B8E8`); hdr+0x1C → `D_800B1624` (`0x8006B90C`);
   hdr+0x20 → `D_800B1628`.
5. Later `func_8006C118` (inside `6BECC`, not `6B4F8`) writes
   `D_800B0E70[0]` from the CE2=14 package hdr+0x0C rel24.
   Writer B @ `0x8006C140` fills the type-0 command row, including
   command 4.

`func_80035038` copies `D_800B0E70[type]` to actor `+0x1AC` when
`type<10`. Empty `+0x1AC` ORs `+0x98` with `0xE0` and skips
`1A680` / `362B8` / `3D050`.

`func_8001A680` copies `D_800B0E98[type*192+cmd*4]` to actor
`+0x1B0` and `lbu resource+2-1` to `+0x0F`.

Actor `+0x1B4` is an **in-actor dest object**, not a table pointer.
`15240` uses `dest = actor+0x1B4`. `6CC68` publishes `D254+0x1B4`
to `D_800B0D10`. `362B8` / `3D050` initialize that dest only when
`+0x1AC != 0`. Do not invent a host-side equivalent.

## m0005i hdr+0x0C (`D_800B0E70`)

| idB | chunk2 ptr | size | SHA-256 | consumer |
|---:|---|---:|---|---|
| 2 | `0x90B4` | 23700 | `b68b7c6bf5ee7986…` (full in CSV) | type 2 `+0x1AC` |
| 5 | `0xED48` | 628 | `e6fdc055a9912e18…` (full in CSV) | type 5 `+0x1AC` |

Window `0x8006B7C8..0x8006B820` SHA-256
`f2f5b2382224b409acea599d5bdf1572a082fe30d76d19731ff712b4aef0e2d0`.

## CE2=14 type-0 command bank (`D_800B0E98` row 0)

`D_800930D8[22]=396`, `[23]=428`. Bank 65536 bytes, SHA-256
`db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2`.

25 directory records, 18 unique payloads. Command 4 (first `29810`
bind): ptr `0x6C14`, 1700 bytes, enc=1, bones-1=30, frames=18,
SHA-256 `6207fbca2fe44a3549bf0b7fbcf1ce3e979a0a12e8b130e606ea4a985b1885e4`.

`6C118` type-0 resource object: package `+0x8`, 23864 bytes,
SHA-256 `bcbdf4f4117bda46…` (full in CSV). Window
`0x8006C0E4..0x8006C174` SHA-256
`59c4bd179f4f37cbc881c9d847d80e469f4311f69c5efa904b2c583169cf70f2`.

## Writer A (m0005i hdr+0x10)

21 records. Type 0 commands are `0x18,0x1D,0x1E,0x1F,0x20` only
(no command 4). Type 2 command `0x17` at `+0x1F3A8` has `byte+2=51`,
so `1A680` sets `+0x0F=50`. Type 5 commands `0..3` are 60/88-byte
payloads, not 31-bone clips.

## VM script corpus

`17018` fetch is `word@pc`, kinds-extension `word2@pc+4`, immediates
at `pc+8`, span `8+argc*4`. Opcode = `word & 0x1FFF`. First 24
well-formed words per type are in `BATTLE_SCRIPTS.csv` with window
SHA-256, branch rels (`0`/`5`/`0x12` as `imm<<1`), and yield ops
(`1`/`2`/`0x1F`/`0x20`/`0x30`/`0x64`/`0x9C`).

Upcoming / still-open on the live streams (do not implement here):

- type 0 after `0x9B`: `0xED` `0xA29`, then `0x14`×2, `0x0B`…
- type 1 prefix `0xEA` then three `0x08` spawns
- type 3 after first `0x02`: `0x5E` / `0x77` (pose copy + edge test)
- type 6 after `0xCE`: `0xEA` (AKAO-adjacent; not named here)

## Destination / reload

| token | name | table | rel | packed | sectors | full SHA-256 |
|---|---|---:|---|---|---|---|
| `0xA80002C8` | M0005I | 4 | `0x266A` | `0x0600A921` | 33+169+96 | `fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724` |
| `0xA80663C8` | M0367I | 366 | `0x15050` | `0x04E0AA21` | 33+170+78 | `671d8c53b0c12b13e1580b844e37c51dc6120b6cf7af78472037d7ed1a7da6de` |

m0005i chunk2 SHA-256
`01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b`.
M0367I chunk2 SHA-256
`ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e`.

`6B4F8` is the sole TEXT caller from `3F074@0x8003F088`. `6B35C`
clears the publish slots before that load. M0367I Writer A has 36
rows; hdr+0x0C has idB `2,3,4`. Spawn-list types for M0367I are
not yet walked.

## Files

| file | contents |
|---|---|
| `ACTOR_RESOURCES.csv` | 7 listed types: script, model, writers, vtable |
| `BATTLE_SCRIPTS.csv` | 17018-decoded prefix per type |
| `RESOURCE_PUBLICATION.csv` | 6B35C → 6B4F8 → 6C118 order |
| `PEIMG_PACKAGES.csv` | dest packages + unique clip payloads |
| `UNKNOWN_DEPENDENCIES.csv` | 3D050-gated body, +0x1B4 init, absent B0E70 rows |

## HUMAN_VERIFY

- Confirm CE2=14 `6C118` object at `+0x8` is the type-0 `+0x1AC`
  payload the main lane should consume, not m0005i hdr+0x0C.
- Confirm actor `+0x1B4` stays an in-actor dest, not a PE.IMG pointer.
- Do not bind slot tags 1332/1333/1334 to a species name.

Next precovery cut: M0367I 12574 list + hdr+0x0C payloads, then
type-0 stream command immediates against the CE2=14 bank.
