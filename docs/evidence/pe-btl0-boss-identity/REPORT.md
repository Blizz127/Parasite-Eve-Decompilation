# PE-BTL0-BOSS-ID — Day 1 enemy identity from retail strings

```text
PE-BTL0-BOSS SUCCESS — DAY 1 ENEMY IDENTITY RESOLVED FROM RETAIL STRINGS
```

Evidence only. No battle implementation. No ATB / damage / PE. No
production runtime edit. No push. Walkthrough, wiki, and OST names
were not used.

```text
authorities
  PE-BTL0  de959cf
  PE-TXT0  1d47df3
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
exe_sha1    = 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
m0005i_pkg  = sha256 fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724
tool        = python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --scene m0005i --message 0x2E
```

## 1. Name resource: same stream 1

m0005i slot7 store 1 is byte-identical to the TXT0 USA English bank:

```text
slot7[0]  size 0x1956  sha256 467bf214…eba1   stream 0 (not USA)
slot7[1]  size 0x2A25  sha256 231da625…262a   stream 1 (USA)
```

USA boot still ORs `D_800B0CD8 |= 0x40000000` and binds `D_800B162C`.
Lookup is unchanged: `func_80037870` scans `(FF|F9) FE <id>` and
letters are `code + 0x31`.

There is no second field-package name bank on this room. The boot
table `func_8005DC4C` (120 records, indices 3..117) is the TXT0
item/system archive. 1332 is far outside that count, so it is not
a `func_8005DC4C` name index.

Battle cluster `0x80029800–0x80031000` has **zero** `jal` to
`func_80037870` / `func_800375E0` / `func_80017410`. Field dialogue
is how this map names the combatant. The battle tick does not open
stream-1 IDs.

A small EXE rodata run at `0x80091496` decodes with the same letter
map to status lines (`Susceptible to poison`, `Failed to escape`).
Those are not enemy names and are not indexed by 1332/1333/1334.

## 2. 1332 / 1333 / 1334 are slot resource halfwords

Opcode `0x5A` (`0x80018164`) calls `func_80030220` when
`*(D_8009D2F0)+0x0C != 0` (the post-`0x6F` slot path). That function
is a jump table at `0x80010C90`, index `tag - 40`.

| tag | store | first-encounter value | consumer |
|---|---|---|---|
| 50 | `sh` actor/slot `+0xB0` | 1333 | compared to 1 at `0x80087EC4` (1333 ≠ 1) |
| 51 | `sh` actor/slot `+0xB2` | 1332 | `lhu` at `0x8002FC14` → `func_8006DCE4` → `func_8006DED4` via `D_800B0E64` |
| 52 | `sh` actor/slot `+0xB4` | 1334 | spatial halfword (`0x8003058C` angle; `0x80087EB0` increment) |

1332 is an **archive/model key**, not a message ID. Stream 1 contains
no `u16`/`u32` 1332/1333/1334 and no `(FF\|F9) FE` marker for those
values (markers are one byte).

## 3. First encounter — opened names

Module 2, persist `[1]==4` path, last `0x0D` before the `0x1A` /
`0x6F` / first `0x89`:

```text
+0x1C3C  0x0D  id=46   "Actress, Hahaha..."
+0x1C48  0x22  id=46
+0x1CC8  0x0D  id=50   "Actress, Our bodies are communicating with each other..."
+0x1CF4  0x22  id=50
+0x1DCC  0x1A  local[24] = rng(0,100)
+0x1E34  0x6F
+0x223C  0x5A  50=1333, 51=1332, 52=1334
mod6 +0x350C  0x89
```

Speaker tokens use the same comma convention as TXT0 (`Prince,`,
`King,`, `Man,`). The opened first-encounter speaker is **Actress**.

Stream 1 also holds `id=45` `Battle VS Eve<01>`. No `0x0D 45` exists
on m0005i. A full field-table census finds that ID on later maps
only (first: m0038i). It is not the first-fight open.

## 4. Day 1 boss is this room, later named Eve, same slot

Do not treat the first `0x89` as a boss by position. The later
m0005i requests (`mod6+0x3728`, `+0x414C`) have **no** `0x6F` /
`0x5A` / `0x70` / `0xB7`. They reuse the module-2 slot
(1332/1333/1334). They are not a second species.

Same-room stream-1 lines that name **Eve** (module 0, after persist
tests, not the first `0x89` preface):

```text
id 54   Actress, Eve...
id 55   [NAME], Eve...
id 56   Eve, I'm surprised you don't know me...
id 132  Eve, Hahaha...
```

id 54 still opens as `Actress,` then the body letters `Eve`. id 56
and 132 open as `Eve,`. That is the same bank, same room, later
speaker token — not a later map and not a new formation.

`m0012i` / `m0013i` / `m0014i` / `m0016i` `0x89` sites remain off
the SYS0 first-play prefix and are not claimed as Day 1.

## 5. Formation 49 vs 50

`local[24] = rng(0,100); < 19 → 49 else 50`. The 1332/1333/1334
stores are immediates on both arms. No later first-encounter command
reads `local[24]` before module 6 overwrites it (`+0x380C = 0`, then
`0x94` dest). See `FORMATION_SELECTION.md`.

## 6. RNG

The 49/50 write is RNG-dependent. The `0x89` store is not. Pin the
`func_80070D6C` word at the `0x1A`, not at `0x89`. See
`RNG_DEPENDENCE.md`.

## 7. Files

```text
docs/evidence/pe-btl0-boss-identity/REPORT.md
docs/evidence/pe-btl0-boss-identity/ENEMY_STRING_TABLE.csv
docs/evidence/pe-btl0-boss-identity/FORMATION_SELECTION.md
docs/evidence/pe-btl0-boss-identity/RNG_DEPENDENCE.md
```

Reproduce the name decode with the existing TXT0 tool. The bank is
the shared stream-1 blob, so m0004i is a valid package:

```text
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --scene m0004i --message 0x2E
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --scene m0004i --message 0x32
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --scene m0004i --message 0x2D
```

---

```text
base_btl0_commit=de959cf
txt0_commit=1d47df3

enemy_name_resource=SHARED_STREAM1
enemy_name_lookup_status=PROVEN_FUNC_80037870
enemy_name_decode_status=PROVEN_LETTERS_CODE_PLUS_0x31

first_encounter_enemy_ids=46,50
first_encounter_enemy_names=Actress

day1_boss_identity_status=PROVEN_STREAM1_EVE_SAME_M0005I_SLOT
day1_boss_encounter_location=m0005i_same_room_later_dialogue_same_slot_0x89
day1_boss_string_ids=45,54,55,56,132

formation_variant_selection=0x1A_0_100_SLT_19_THEN_49_ELSE_50_INTO_LOCAL24_UNUSED_BY_SETUP
formation_rng_dependent=yes
rng_word_to_pin=func_80070D6C_u32_low16
rng_pin_site=m0005i_mod2_+0x1DCC

btl1_determinism_ready=YES

hard_blockers=
unknowns=whether battle HUD ever opens id 45; whether any untranslated battle reader consumes actor local[24] before +0x380C overwrite; archive payload behind 1332
warnings=1332_1333_1334_are_not_string_ids; do_not_use_walkthrough_names; first_0x89_is_not_a_later_map; 0x1A_is_generic_rng; id_45_Battle_VS_Eve_not_opened_on_m0005i; later_0x89_reuse_same_slot

SUCCESS

PE-BTL0-BOSS SUCCESS — DAY 1 ENEMY IDENTITY RESOLVED FROM RETAIL STRINGS
```
