# PE-BTL128 — type-6 wait vs +0x1850 is a task split

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
m0005i chunk2 SHA-256 `01a64ba3…7e3b`. No matching `src/` C.
Do not poke `D_800B6A80`. Do not inject mailbox `0x7F` / `0x81`.

## The apparent contradiction is a task-model error

Linear file order put type-6 `+0x1850` `0x2A[scratch[0],2]`
"after" the `+0x190` wait. CFG on `0x00` / `0x05` / `0x1D`
rejects that:

| Start | Reaches `+0x190` | Reaches `+0x1850` | Reaches `+0xFC8` `0x55` |
|---|---|---|---|
| script entry `0` | yes | no | no |
| wait escape `+0x1E8` | via loop | no | no |
| mailbox `+0xD08` | no | yes | yes |

`+0x1850` is a **different task**. The main task never
executes it.

```text
main  +0x000 0xCE … +0x180 0x14 → actor+0x19C = base+0xD08
      +0x190 wait scratch[0]&4
      +0x1E8 0x12 … +0x394 0x89 … +0x3F8 0x1C(2,0,0x7D) … +0x40C 0x20
mail  +0xD08 0x1F … +0xFC8 0x55 … +0x1100 0xAE … +0x1850 0x2A[0,2]
```

`+0xD00` `0x20` is dead linear padding, not a main-task PC.
`+0xFAC` is `0x2A` bit 3, not `0x55` (that is `+0xFC8`).

## Who can start the mailbox

`0x1C` extra is always 0. Delivery is type+id through
`65400` → `12700(actor+0x19C)`. Type-6 inbox on m0005i:

| Sender | Rel | Payload |
|---|---|---|
| type 0 | `+0xCAC` | `0x81` |
| type 0 | `+0x113C` | `0x70` |
| type 2 | `+0x534` | `0x84` (`0x7D` handshake) |
| type 2 | `+0x5A8` | `0x7B` |
| type 2 | `+0x624` | `0x86` |
| type 2 | `+0x6E4` | `0x7C` |

No dest-enter `0x1C` to type 6. Type-1 `0x08` types are
`3,0,5,2,4` — no second type 6.

## First-visit scheduler while type-6 waits

125E0 desc is type 1 then type 6. Type-1 then:

```text
+0x198 persist[0x4A]==39
!=39 → +0x1CC 0x86 / +0x1D8 0x1C(0,0,0xFF) / +0x1F8 0x20
==39 → skip the 0xFF send
```

New-game persist 0 takes `0xFF`. Type-0 mailbox `+0x618`
`0x1F` compares 255 and `0x00` to `+0x2F4`. persist `!=39`
skips to `+0x608` `0x20`. That arm does **not** reach
type-0 `+0xCAC` `0x1C(6,0,0x81)`.

Live dest-ready + 16 ticks: type-6 still at `+0x1DC`,
`+0x19C=+0xD08`, slots `A0/A4` empty, no prepended
mailbox task, `scratch[0]&4` clear. Type-1 parked after
`0xFF`. Type-0/2 parked. Type-2 body still 0.

## Retail 3F074 order

Once `3F074` is entered there is no branch before
`1266C`. Order is:

```text
6B35C → 6B4F8 → 6BD68 → D224=1 / D308=1 → 34FC4 → 1266C
→ 3F758/3F798/68B94/79024/CFEE|=0x40
→ 6BE4C → 6BECC* → 6C4C4(CE4) → 6C5BC* → 1A918 → 371B0 → 125E0
```

Native dest-ready had `6BECC/6C5BC/1A918` **before**
`1266C`. That cannot invent a scratch writer (those
leaves do not store `B6A80`), but it made `6BE4C` see
post-Writer-B `CE3`. Dest-ready now matches the
`1266C → 6BE4C → 6BECC → 6C4C4 → 6C5BC → 1A918 → 125E0`
tail. `6BD68` / lighting / `371B0` stay deferred.
Live dest-enter `CE3=0` so `6BE4C` does not set
`+0x0E` bit 2; `6BECC` later copies `CE3=CE2`.

## Hypotheses

| Claim | Result |
|---|---|
| `1266C` skipped on dest-enter | REJECTED (`3F074` always jals it) |
| `+0x1850` is later on the wait task | REJECTED |
| same actor, two tasks, different PCs | PROVEN (main vs `+0xD08`) |
| type-1 `0x08` makes another type 6 | REJECTED |
| dest-ready order ≠ retail | PROVEN; native tail reordered |
| first-visit mailbox starts `+0xD08` | REJECTED (no live `0x1C` to type 6) |
| scratch set before type-6 wait | REJECTED on native dest-ready |

`35558@35C1C` jals `36448` (deferred). That leaf is a
proximity walk: if `actor+0x1A0` is set it `12700`s that
entry into slot `A4`. Type-6 never writes `+0x1A0` (only
`+0x19C`). Type-5 `+0x1A0=+0xF8` and type-0/2 `+0x1A0`
are `0x20` parks or the type-5 `0x1C(0,0,0xFE)` island.
Proximity spawn is not dest-enter and does not reach
type-6 `+0x1850` without a later `0x1C` to type 6.

PCSX watch: `pc_port/tools/pe_btl128_ordering.lua`.
First-visit retail recording still needs a human new-game
drive; the Theater memcard is not m0005i.

## Verify

```text
python3 pc_port/tools/pe_btl128_task_cfg.py
PE_TEST_FILTER=BTL128 ./pc_port/build/pe-native-tests
```
