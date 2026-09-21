# `m0377i` destination contract — research note

Status of the open unknown recorded as item 6 in
`docs/ai_context/BOOT_TO_DAY2_COVERAGE.md` §6 and
`PE_DAY1_ACCEPTANCE_CONTRACT.md:163,190`:

> **`m0377i` destination contract** — identity proven, forward contract unknown.

This note closes the *identity* half with an address-exact transfer table and
records exactly what is and is not proven about the *forward* contract. It does
not claim a live traversal; it states the bytecode-visible edges.

## 1. Identity (proven)

| Fact | Evidence |
| --- | --- |
| Field-table index `376` is `m0377i`, package `start=87504..87603`, `meta=0x01902921` | `pe_pst0_scan.parse_field_table` over the SHA-1-exact EXE |
| Script base VA after load | `0x80194FF4` (`0x8018EFE8 + off - (c0+c1)*2048`, the same base rule the transition oracles use) |
| Transfer token | `0xA80673C8` — the packed name decodes to `m0377i` (`pe_btl147_theater_eve_path` scan, entry `packed_name: "m0377i"`) |
| Script sha256 | `a70d196cc56d398c58d3d542a46c988651e0e8b1193518c357be3bd891663a40` |

The prior scan's `scene_name_from_index`/`map_id_from_low` columns show
`m0231i` for this token; that column is an *index-arithmetic* artifact
(`map_id` is read from the token's low bits, which do not encode the name for
this range). The identity authority is the packed token name, which is
`m0377i`, not `m0231i`.

## 2. Forward edges (bytecode-visible)

`m0377i` (6 modules) has exactly one transfer:

| Module | PC | Command | Destination |
| --- | --- | --- | --- |
| 5 | `0x80195768` | `0x31` token `0xA8067448` | **`m0378i`** |

The transfer is preceded in module 5 by the `0x0A` `[persist[1] = 377]` write
at `0x80195758` and a `0x05` local guard; it is followed by a `0x20` yield.
So `m0377i`'s only *outbound* edge is `m0377i -> m0378i`.

## 3. Inbound edge (the contract that made it interesting)

`m0378i` (index `377`) module 4 (`0x80195638..0x8019583C`) contains a two-way
branch on the story word:

| PC | Command | Effect |
| --- | --- | --- |
| `0x801956DC` | `0x09` sub `0x0A` (`sa < sb`) with operand `persist[74]` and const `0x30` | compare the story word with `0x30` |
| `0x801956F4` | `0x05` guard on the local result | selects an arm |
| `0x80195718` | `0x0A` `[persist[1] = 378]` | |
| `0x80195728` | `0x31` token `0xA80673C8` | **`m0377i`** |
| `0x80195754` | `0x0A` `[persist[1] = 378]` | |
| `0x80195764` | `0x31` token `0xA80000C8` | **`m0001i`** |

`persist[74]` is the same story/progress word the Day-1/Day-2 transition note
uses (`0x800A7918`), so this is the Day-1 progression gate the BTL0
field/battle-handoff lane already documented: `m0378i` can loop back to
`m0377i` or fall through to the `m0001i` map (re)entry.

`m0377i` module 5 (`0x801956A8..0x80195798`) then unconditionally transfers
back to `m0378i` at `0x80195768`. Net effect: **`m0377i` is a bounce room
between two `m0378i` arms**, not a doorway to a new area. That is consistent
with the earlier negative finding that `m0377i` contains none of the battle
opcodes
(`docs/evidence/pe-btl0-field-battle-handoff/FIRST_DAY1_ENCOUNTER.md`).

## 4. What this does and does not close

**Closed:** the forward contract is no longer unknown at the bytecode level —
`m0377i -> m0378i` (only), with `m0378i -> {m0377i, m0001i}` as its inbound
context. The "destination contract" is a two-room bounce, and no field-table
room beyond `m0378i` is reachable from `m0377i` in one hop.

**Still open (not proven here):**

- Which `m0378i` arm runs on first play. That needs the live `persist[74]`
  value at that frame and the actual `0x05` guard outcome; the
  `pe_btl147` `last_persist_4a`/`last_persist_1` columns are *nearest
  preceding write* annotations, not guards.
- The interaction/runtime semantics of the `0x77` volume and the `m0377i`
  module-0/1/2/3/4 setup blocks (camera, slot, effect) are not part of this
  contract note.
- The first-play camera/`0x82`-application question is tracked separately as
  `DEBT-FID1-002` in `docs/evidence/pe-debt1-fidelity-registry/`.

## 5. Reproduce

```
python3 pc_port/tools/pe_field_vm_opcode_coverage.py    # decodes m0377i/m0378i (and friends)
python3 pc_port/tools/pe_day1_day2_transitions.py       # room-transfer graph + assertions
```

Requires the Disc 1 image (`local/pe_disc1.path`) and
`build/disc1.candidate.exe` (sha1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
