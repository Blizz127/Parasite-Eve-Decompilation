# PE-PST0 REPORT — retail persist[] provenance

```text
PE-PST0 SUCCESS — RETAIL PERSISTENCE PROVENANCE REGISTRY ESTABLISHED
```

Evidence-only. No production state redesign. No save implementation.
No push.

## Isolation

```text
workspace = /var/home/blizz/dev/parasite-eve-port-black
branch    = phase6e-b-provider-frontier
tool      = python3 tools/research/pe_pst0_scan.py "$PE_DISC1_BIN"
disc      = Parasite Eve (USA) (Disc 1).bin
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha1    = 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
pe_img_sha1 = 146c0ce7308bf9fdc2ba5a84230e198db0663f3b
```

## 1. Storage

Canonical `persist[]` is **`D_800A77F0`**, 512 × 32-bit words
(`0x800` bytes). Binder mode 2. New-game zero is `func_80034F10`
(`sltiu` count `0x200`), called from boot `func_8003E680`.
Field load `func_80034FC4` does not touch it.

Four other banks share “local/persist” vocabulary in Python and
must stay separate: actor locals (mode 1), cond `D_8009DF70`
(mode 3), scratch `D_800B6A80` (mode 4, 64 words), immediates
(mode 0). Only mode 2 is the save-backed story bank.

Details: `STORAGE_LAYOUT.md`.

## 2. Binder

Field scripts address persist by putting **mode 2** on an
argument of `0x0A` (`*dst=*src`) or `0x09` (24-entry ALU).
Index is a word, then `<< 2`. Relational compares on
`persist[0x4A]` are signed `slt`. Equality on `persist[1]` is
xor/bit-exact. Flag tests on `persist[0]` are `and`/`or`.

Details: `BINDER.md`.

## 3. Reader/writer inventory

152 persist accesses decoded from scripts in:

| Scene | Table | Status | Accesses |
|---|---:|---|---:|
| m0001i | 0 | Day-1 extra | 13 |
| m0002i | 1 | current route | 11 |
| m0003i | 2 | current route | 14 |
| m0004i | 3 | current route | 21 |
| m0005i | 4 | Day-1 extra | 75 |
| m0372i | 371 | current route | 7 |
| m0377i | 376 | current route (now available) | 3 |
| m0378i | 377 | current route | 8 |

Full dump: `READERS_WRITERS.csv`.

Observed indices: `0, 1, 8, 0x0A, 0x12, 0x18, 0x19, 0x1A, 0x4A, 0x50, 0x54, 0x64`.

## 4. Current-route timeline (re-derived)

New-game persist is all-zero (`func_80034F10`). First-play mutations
that the scanner can attribute without guessing mailbox occupancy:

| scene | module | PC | index | before | op | after | consequence |
|---|---:|---|---:|---:|---|---:|---|
| m0002i | 0 | `+0x0268` | 0 | 0 | assign | 0 | rewrite 0 |
| m0002i | 0 | `+0x0278` | 0x4A | 0 | assign | 9 | sidewalk gate |
| m0002i | 5 | `+0x0EE4` | 1 | 0 | assign | 2 | then `0x31 m0003i` |
| m0003i | 1 | `+0x0718` | 1 | 2 | assign | 3 | leftover source id |
| m0372i | 1 | `+0x04E4` | 0x4A | 9 | assign | 0x12 | then `0x31 m0004i` |
| m0372i | 3 | `+0x0D48` | 0x4A | 9 | assign | 0x12 | same value |
| m0004i | 0 | `+0x0534` | 0 | 0 | assign | 0 | rewrite 0 |
| m0004i | 0 | `+0x0544` | 0x4A | 0x12 | assign | 0x18 | concert gate |
| m0004i | 4 | `+0x0F30` | 1 | 3 | assign | 4 | then `0x31 m0378i` |
| m0378i | 4 | `+0x0724` | 1 | 4 | assign | 0x17A | then `0x31 m0377i` |

Verified (not copied) values:

- `persist[0x4A]`: write 9, 0x11 (m0372i type-2, **not** first-play token pair), 0x12, 0x18; compares `<9`, `<0x10`, `<0x18`, `<0x28`, `<0x30`, `>=0x11`, `>=0x18`, `>0x78` (`slt` subop 9).
- `persist[1]`: writes/tests 1, 2, 3, 4, 5, 9, 0xA, 0x179, 0x17A, 0x3E7.

Alternate hops recorded but **not** first-play: m0004i→m0005i,
m0378i→m0004i, m0378i→m0001i (`0x4A >= 0x30`), m0377i→m0378i.

After the first-play walker: `{0:0, 1:0x17A, 0x4A:0x18}`.

CSV: `CURRENT_ROUTE_TIMELINE.csv`.

## 5. Classification

| index | confidence | semantic_name | domain |
|---:|---|---|---|
| 1 | **PROVEN_SEMANTIC** | `entrance_selector` | equality set of source-map / door ids; writers sit next to `0x31`; dest modules `==` the same slot |
| 0x4A | **STRONG_ROLE** | *(none)* | compared/assigned thresholds 9/0x11/0x12/0x18/0x28/0x30/0x78; **not** named storyProgress |
| 0 | **PROVEN** | *(none)* | flag word; bit 2 (0x4) set by m0360i ONLY, bit 1 (0x2) by 18 scenes |
| 0x18, 0x19, 0x1A | VALUE_ONLY | | bit/eq/add on Day-1 maps |
| 8, 0x0A, 0x12, 0x50, 0x54, 0x64 | VALUE_ONLY | | m0005i only |

`entrance_selector` is allowed because **seven** scenes write or
equality-test slot 1, and dest spawn arms are those tests.

## 6. Value domain

See registry `known_values` / `bit_masks` / `operations`.
Do not type a slot from one store: `persist[0]` is assigned 0
and also masked; `persist[0x4A]` is assigned and compared.

## 7. Reset

Only proven clear: `func_80034F10` at boot/new-game.
Room change survives. Day-transition reset: **UNKNOWN**.

## 8. Save boundary

Serialized persist range: **`D_800A77F0` … `+0x800`**.
Parent `func_80040B80` prefixes `0x12E4` then appends `0xA8`
sibling words (token, inhibit, CE0-CE6, …). CRC-16 poly `0x1021`
with table `D_8009EED0` runs after the copy; span/header/slot
file format **not** closed. See `SAVE_BOUNDARY.md`.

## 9. Cross-system

EXE `lui/addiu D_800A77F0` sites = 5:

1. binder mode 2
2. `func_80034F10` zero
3. `func_8003F800` save
4. `func_8003FBD8` load
5. `func_80053128` — reads **other** index ranges
   (`28-33, 49-53, 58-63, 68-73, 90-96, 98-99, 263-300, 312-337`)
   and if `persist[i] - 0x100 < 0x80` sets `D_800C0EB1` bit 3

Day-1 slots 0 / 1 / 0x4A are **not** in those ranges.
No other battle/menu persist base was found.

## 10. Fidelity-debt

Highest existing shim: short persist vectors + “0 means 9” +
token-only hops. Unsafe once save/battle/m0004i reel/`0x4A`
thresholds exist. See `FIDELITY_RISKS.md`.

## Files

```text
docs/evidence/pe-pst0-persist-provenance/REPORT.md
docs/evidence/pe-pst0-persist-provenance/STORAGE_LAYOUT.md
docs/evidence/pe-pst0-persist-provenance/BINDER.md
docs/evidence/pe-pst0-persist-provenance/PERSIST_REGISTRY.csv
docs/evidence/pe-pst0-persist-provenance/CURRENT_ROUTE_TIMELINE.csv
docs/evidence/pe-pst0-persist-provenance/READERS_WRITERS.csv
docs/evidence/pe-pst0-persist-provenance/SAVE_BOUNDARY.md
docs/evidence/pe-pst0-persist-provenance/CROSS_SYSTEM_READERS.csv
docs/evidence/pe-pst0-persist-provenance/FIDELITY_RISKS.md
docs/evidence/pe-pst0-persist-provenance/CODE_CROSSREF.csv
docs/evidence/pe-pst0-persist-provenance/PST1_IMPLEMENTATION_CONTRACT.md
tools/research/pe_pst0_scan.py
```

## Status block

```text
persistent_storage_status=PROVEN
persistent_storage_base=0x800A77F0
persistent_storage_size=0x800
element_width=32

binder_status=PROVEN

entries_observed=12
entries_semantically_proven=1
entries_value_only=9

current_route_coverage=m0002i,m0003i,m0372i,m0004i,m0378i,m0377i

persist_0_status=STRONG_ROLE_FLAGS
persist_1_status=PROVEN_SEMANTIC_entrance_selector
persist_0x4A_status=STRONG_ROLE_UNNAMED_GATE

room_transition_survival_status=PROVEN_SURVIVES
battle_survival_status=NO_CLEAR_FOUND
day_transition_status=UNKNOWN

save_range_status=PROVEN_0x800
save_serialization_status=PARTIAL_HEADER_CRC_OPEN

cross_system_reader_status=PROVEN_FIVE_EXE_SITES

persistence_registry_ready=YES
save_contract_ready=NO
ue_persist_model_ready=NO

highest_risk_existing_shim=8-slot persist vector plus persist[0x4A]==0 treated as 9 plus token-only hops

pst1_implementation_ready=NO
sav0_research_ready=PARTIAL

hard_blockers=save header/CRC span/card block map not closed; day-transition reset not found; battle keep is absence-of-clear not a positive proof
unknowns=func_8003708C/370A8 ALU helpers; 0x12E4 save prefix layout; D_800B8A20/B0CB0/9D1B0 sibling blocks; m0005i slot semantics; day boundary
warnings=do not name persist[0x4A] storyProgress; do not merge scratch/cond/locals; m0372i persist[0x4A]=0x11 is a type-2 writer not on the first-play token pair; persist[1]=3 is a leftover on m0004i first arrival

SUCCESS

PE-PST0 SUCCESS — RETAIL PERSISTENCE PROVENANCE REGISTRY ESTABLISHED
```
