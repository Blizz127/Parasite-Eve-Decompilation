# PE-BTL0 — Day 1 field→battle handoff and first combat identity

```text
PE-BTL0 SUCCESS — DAY 1 FIELD→BATTLE HANDOFF AND FIRST COMBAT CONTRACT ESTABLISHED
```

Evidence only. No battle implementation. No production runtime edits.
No push.

```text
authorities
  PE-SYS0  c799bbb
  PE-PST0  6f51289
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
tool        = python3 tools/research/pe_btl0_scan.py "$PE_DISC1_BIN"
```

This rung does **not** reverse ATB, damage, or PE. It proves the
smallest retail boundary:

```text
field runtime  ->  battle request  ->  battle resources/state  ->  battle entry
```

and identifies the earliest Day 1 combat room reachable from the
SYS0 first-play prefix.

## 1. Entry

Field scripts request battle by writing **`D_8009D28C = 6`**.

| Item | Value | Proof |
|---|---|---|
| Opcode | `0x89` | `D_800910A0[0x89] = func_80017FF0` |
| Handler | `func_80017FF0` | matching leaf `src/func_80017FF0.c`: `D_8009D28C = 6; return 1` |
| Mode word | `D_8009D28C` / `$gp+0x51C` | PST0/5EJ int-state; codes 0/3/4/5/6/8 |
| Consumer | `0x800299F4` | `lw $gp+0x51C`; `addiu $v0, 6`; `bne` |

`0x89` is **not** inferred from a name. The handler is a matching C
leaf. The consumer compares the same word to literal 6 inside the
`0x80029818` / `0x800299CC` cluster.

Related mode opcodes (same word, not all are battle):

| Opcode | Handler | Store |
|---|---|---|
| `0x89` | `func_80017FF0` | 6 — battle **request** |
| `0x8A` | `func_80017FDC` | 5 |
| `0x95` | `func_800192B8` | 0 — field / clear |
| `0x96` | `func_800192C8` | 8 |
| `0x94` | `func_80019154` | read into dest (script poll) |

After `0x89`, m0005i polls `0x94` until the word compares equal to
**7**, then continues (inhibit, `0xAA`, mailbox). The EXE writer that
stores 7 was **not** one of the four matching setters. Victory/exit
code `7` is observed from the script immediate only.

Details: `FIELD_ENTRY.md`.

## 2. First Day 1 encounter

Proven first-play prefix has **zero** `0x89` / `0x6F` / `0x70` / `0xB7`
on `m0002i`, `m0003i`, `m0372i`, `m0004i`, `m0378i`, `m0377i`.

The earliest combat-capable room on the SYS0 route is **`m0005i`**
(table index 4).

```text
m0004i first-play reel (persist[0x4A] 0x12 -> 0x18, opcode 0x3F)
  -> type-0 mailbox poll +0x05D0
  -> module 4 volumes +0x0F78 / +0x10E4
  -> 0x1C send mailbox 3 or 4
  -> poll arms local[4]==3 or ==4
  -> persist[1]=4 ; 0x31 0xA80002C8 m0005i
  -> m0005i persist[1]==4 entrance
  -> module 2: 0x1A RNG then 0x6F / 0x5A / 0x70 / 0xB7
  -> module 6 +0x350C 0x89  (first mode-6 request)
```

North volume on the same m0004i module (`+0x0EB0`) still publishes
`m0378i` (SYS0/RD6-A). That is exploration, not combat. Combat is
the **south/side volumes** that send mailbox 3/4.

Trigger class: **scripted volume + mailbox**, not a random walk
table. A 0–100 `0x1A` roll **inside** m0005i only picks variant
local `49` vs `50` (`< 19` → 49, else 50) before the same `0x6F`
setup. The encounter is not avoidable if Day 1 is to continue;
it is avoidable only by never walking those m0004i volumes.

Details: `FIRST_DAY1_ENCOUNTER.md`, `ENCOUNTER_TABLE.csv`.

## 3. State transfer

| Surface | Battle reads? | Battle writes? | Notes |
|---|---|---|---|
| `persist[]` `D_800A77F0` | **no direct EXE site** | **no** | PST0 five-site census unchanged inside `0x80029800–0x80031000` |
| Aya actor `D_8009D254` | yes | yes (via existing actor ops) | battle init `lw D_8009D254`; `0x5A` writes through Aya object |
| Slot table `D_800A5D58` 7×220 | yes | yes | `0x6F` allocate; `0x70`/`0xB7`/`0x5A` fill |
| Mode `D_8009D28C` | yes | yes | request 6; runtime may store 0/3/4; script waits for 7 |
| Field pose / map token | not copied by `0x89` | n/a | scene stays `m0005i`; no `0x31` on the request |
| RNG | script `0x1A` before setup | battle cluster has **zero** jal to `70D10/70D6C/70DD0` | |

HP/MP/PE/equipment/inventory fields inside the 216-byte slot body
are **not** decoded this rung. `0x5A` is a tagged setter
(`func_8002FF78`) switching on the first argument (0,1,3,5,6,10,11,…).

Details: `STATE_TRANSFER.md`.

## 4. Resources

First-encounter numeric IDs written on m0005i module 2 after `0x6F`:

```text
0x5A[50]=1333
0x5A[51]=1332
0x5A[52]=1334
0x70(0,0,8,9,1,5,-1,-1)  one operand from cond
```

These are **resource/slot field IDs**, not English enemy names.
Package:

```text
scene          m0005i
table_index    4
PE.IMG         [0x266A, 0x2794) sectors
bytes          610304
sha256         fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724
loader         same field package loader as SYS0 rooms (func_8006B4F8 family)
```

No AKAO handle is bound on the `0x89` itself. m0005i uses `0xEA` near
the request; sequence identity is AUD's problem, not claimed here.

Details: `BATTLE_RESOURCES.csv`.

## 5. Runtime lifecycle

Top of the consumer cluster:

| Phase | VA | Observation |
|---|---|---|
| init / zero | `func` at `0x80029818` | zeros `D_8009D28C`, `D_8009D290`, several gp bytes; jal `func_80020EFC`, `func_80071A64`, `func_800293F4`, `func_800209F0`, `func_80030640`, `func_800339A0`, `func_8001A680` |
| request edge | `0x800299F4` | if `D_8009D28C==6`, store 0 and set `gp+0x10C` |
| active | `0x800299CC` body | large frame (−456); branches on mode and actor+0x4C bits |
| mode 3 / 4 | `0x8001F41C` / `0x80021F04` | internal stores 3 and 4 |
| script-observed done | wait `D_8009D28C==7` | setter of 7 not in matching leaves |
| field restore opcode | `0x95` → 0 | used on m0005i after fights |

ATB/commands are inside this cluster and are **out of scope**.

Details: `BATTLE_LIFECYCLE.md`.

## 6. Return

Battle does not `0x31` away from m0005i. The field script resumes on
the same module after the mode poll. Position is whatever the field
actor still holds (`D_8009D254`). EXP/BP/reward interface is the
`0x5A` / persist writes **after** the wait (m0005i touches
`persist[0x0A]`, `persist[0x12]`, `persist[0x50]`, `persist[0x64]`,
`persist[0x54]`) — formulas not decoded.

Details: `RETURN_CONTRACT.md`.

## 7. RNG

- Script opcode `0x1A` = `func_800176FC` → `func_80070D6C` (equal
  bounds) or `func_80070DD0` (range). Generic, not battle-only.
- First m0005i setup: `0x1A local[24], 0, 100` then `< 19` picks
  49 else 50.
- Battle cluster `0x80029800–0x80031000`: **no** jal to the RNG
  trio. Deterministic entry after the script roll does not need a
  further RNG reverse.

Details: `RNG.md`.

## 8. Day 1 boss

`0x89` occurs three times on m0005i (`+0x350C`, `+0x3728`,
`+0x414C`) and later on `m0012i` / `m0013i` / `m0014i` / `m0016i`.
No formation is proven to be a boss. SYS0 boss blocker stays open.

Details: `DAY1_BOSS_PROVENANCE.md`.

## 9. Files

```text
docs/evidence/pe-btl0-field-battle-handoff/REPORT.md
docs/evidence/pe-btl0-field-battle-handoff/FIELD_ENTRY.md
docs/evidence/pe-btl0-field-battle-handoff/ENCOUNTER_TABLE.csv
docs/evidence/pe-btl0-field-battle-handoff/FIRST_DAY1_ENCOUNTER.md
docs/evidence/pe-btl0-field-battle-handoff/STATE_TRANSFER.md
docs/evidence/pe-btl0-field-battle-handoff/BATTLE_RESOURCES.csv
docs/evidence/pe-btl0-field-battle-handoff/BATTLE_LIFECYCLE.md
docs/evidence/pe-btl0-field-battle-handoff/RETURN_CONTRACT.md
docs/evidence/pe-btl0-field-battle-handoff/RNG.md
docs/evidence/pe-btl0-field-battle-handoff/TRACE_CONTRACT.md
docs/evidence/pe-btl0-field-battle-handoff/DAY1_BOSS_PROVENANCE.md
docs/evidence/pe-btl0-field-battle-handoff/BTL1_IMPLEMENTATION_CONTRACT.md
tools/research/pe_btl0_scan.py
```

## Status block

```text
field_battle_entry_status=PROVEN
field_battle_entry_handler=func_80017FF0
field_battle_entry_opcode=0x89

first_day1_encounter_status=PROVEN_ROOM_AND_TRIGGER
first_day1_source_scene=m0004i
first_day1_trigger=module4_volumes_+0x0F78_or_+0x10E4_mailbox_3_or_4
first_day1_encounter_id=m0005i_mod2_setup_then_mod6_+0x350C
first_day1_formation_id=VARIANT_49_OR_50_PLUS_SLOT_FIELDS_1332_1333_1334

battle_resource_root_status=NUMERIC_IDS_ONLY
battle_runtime_root_status=PROVEN_CLUSTER

persist_bank_read_by_battle=no_direct_exe_site
persist_bank_written_by_battle=no_direct_exe_site

player_state_transfer_status=ACTOR_AND_SLOT_TABLE_ONLY
rng_handoff_status=SCRIPT_0x1A_BEFORE_SETUP

battle_return_status=SAME_SCENE_MODE_POLL

day1_boss_identity_status=RESEARCH_REQUIRED
day1_boss_formation_status=RESEARCH_REQUIRED

btl1_ready=YES

hard_blockers=day1_boss_unidentified; ATB/damage/PE not in this contract; D_8009D28C==7 setter not located
unknowns=English enemy names; HP/MP/PE slot-body layout; EXP/BP formula; mode 3/4/5/8 full meaning; 0x5A tag dictionary beyond first-encounter tags; m0005i later 0x89 identities
warnings=do_not_use_walkthrough_boss_names; 0x1A_is_generic_rng_not_battle_start; m0377i_is_not_the_first_fight; north_m0378i_is_not_combat; persist_names_still_not_authority; do_not_implement_ATB_in_BTL1

SUCCESS

PE-BTL0 SUCCESS — DAY 1 FIELD→BATTLE HANDOFF AND FIRST COMBAT CONTRACT ESTABLISHED
```
