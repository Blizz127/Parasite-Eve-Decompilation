# PE-DEBT1 — promotion triggers

Triggers name a **rung or system**, never a date. This rung does
not flip any class. SYS0
`FIDELITY_PROMOTION_POLICY.md` remains the stage authority.

A trigger becoming true does **not** auto-promote. The owning
rung must re-measure the retail contract and rewrite the row.

## How to read a trigger

```text
NONBLOCKING_FIDELITY  ->  BLOCKER
    when a later production system shares the approximate state

NONBLOCKING_FIDELITY  ->  PROVEN_EXACT
    when the named retail machinery is implemented and oracled

PARKED_BLOCKER        ->  in-scope work
    when the named rung starts; it cannot ship while the row is open

UNKNOWN_LABEL         ->  named field
    when a consumer proves the gameplay meaning
```

## Per-entry triggers

| id | current class | promote when | becomes |
|---|---|---|---|
| DEBT-FID1-001 | NONBLOCKING_FIDELITY | fade/display interpolation or hop-gate timing is claimed exact (UE0 past m0003i, or Native/UE Parity for the m0004i north hop) | BLOCKER until `0x85`/`0x9C` match RD6-A's 30-tick + `(D_800BCFEE & 3) < 2` gate |
| DEBT-FID1-002 | NONBLOCKING_FIDELITY | the camera-select / `0x82`-vs-auto-apply rule is proven, or UE0/RD7-A loads m0377i | BLOCKER if a runtime applies 52-byte view 0 without `0x82` on m0377i; else named exact rule |
| DEBT-FID1-003 | NONBLOCKING_FIDELITY | a live per-frame camera picker consumes the 16-byte init-table fields | BLOCKER until the entry layout is proven |
| DEBT-FID1-004 | NONBLOCKING_FIDELITY | **BTL1** implements m0004i mailbox 3/4, **or** save/load serializes task/mailbox/actor state, **or** a later first-play script depends on re-arm / `task+0x08` / sender serial rather than the payload byte | BLOCKER (SYS0 §4) until `func_800653B8` / `65400` / `12700` + `D_800A3180` are `PROVEN_EXACT` |
| DEBT-FID1-005 | UNKNOWN_LABEL | a later arm reads `actor+0x23C/23D/23E` as gameplay | named field, or BLOCKER if that arm is on the Day 1 route |
| DEBT-FID1-006 | PARKED_BLOCKER | **TXT1** implements retail glyph draw | stays BLOCKER for Day1 text PASS until VRAM rectangle + pixels are hashed |
| DEBT-FID1-007 | PARKED_BLOCKER | **BTL1** implements return-to-field / the mode-7 store | BLOCKER for any m0005i post-battle script |
| DEBT-FID1-008 | NONBLOCKING_FIDELITY | Native/UE Parity for mix-exact audio | BLOCKER for that parity claim until type-5 reverb is waveform-compared |
| DEBT-FID1-009 | NONBLOCKING_FIDELITY | Day1 Accepted for mix-exact audio | BLOCKER for that claim until the proven SPU interpolator is hashed |
| DEBT-FID1-010 | NONBLOCKING_FIDELITY | collection/art binding proven per used sequence | BLOCKER for mix-exact audio |
| DEBT-FID1-011 | NONBLOCKING_FIDELITY | SFX path and mix bus proven | BLOCKER for mix-exact audio |
| DEBT-FID1-012 | UNKNOWN_LABEL | worker `0x88` payload semantics recovered | labeled command; must not become a BGM selector |
| DEBT-FID1-013 | PARKED_BLOCKER | FMV playback (STR demux + MDEC + XA + sync + return-state) is implemented | BLOCKER for Day1 Candidate (FMV003 is on the first-play arm) |
| DEBT-FID1-014 | PARKED_BLOCKER | cold-boot opcode/overlay map names a hashed STR | BLOCKER for Day1 Candidate if the opening FMV is on the acceptance route |
| DEBT-FID1-015 | UNKNOWN_LABEL | TXT1 maps remaining body codes or the atlas identifies `0x4B` | named glyph; MSG_0x14 cannot be Day1-Accepted with a hole |
| DEBT-FID1-016 | UNKNOWN_LABEL | a battle/menu pad consumer names the Sony bit for mask `0x100` | optional label; the mask contract is already exact |
| DEBT-FID1-017 | NONBLOCKING_FIDELITY | **m0005i / BTL1 / save** uses persist indices greater than `0x4A` | BLOCKER for any Python ` [0]*0x4B ` fixture; UE0/native 512-word banks stay |
| DEBT-FID1-018 | UNKNOWN_LABEL | a writer or a later first-play arm that stores `persist[0x19]` is proven | named cell or BLOCKER if that arm is required |
| DEBT-FID1-019 | PARKED_BLOCKER | a proven Day 1 save/load site exists, or **SAV0** closes the header/CRC/card map | BLOCKER for Day1 Candidate **if** the route requires save; otherwise stays parked |
| DEBT-FID1-020 | PARKED_BLOCKER | a later first-play `0x89` is proven to be the Day 1 boss | BLOCKER for Day1 Candidate until identity + fight exist |
| DEBT-FID1-021 | NONBLOCKING_FIDELITY | a PT package includes the AUD1-D handle timeline | n/a for current AUD1-D runtime; BLOCKER if that old PT is claimed current |
| DEBT-FID1-022 | NONBLOCKING_FIDELITY | m0003i north / m0372i transition implements retail `0xB8` interpolation | BLOCKER when that transition claims skeletal/path fidelity |
| DEBT-FID1-023 | NONBLOCKING_FIDELITY | lobby arrival models `task+0x14` / mailbox `0xFF` for the dest `0x3F` gate | folds into DEBT-FID1-004 once the task machine is exact |
| DEBT-FID1-024 | NONBLOCKING_FIDELITY | a room's live heading leaves the frozen identity | BLOCKER for that room until the retail rsin/rcos table is used |
| DEBT-FID1-025 | UNKNOWN_LABEL | a later arm reads `actor+0x27D` as gameplay | named field |
| DEBT-FID1-026 | NONBLOCKING_FIDELITY | Day1 Accepted for audio records the human speaker verification | required check, not a behavior change |
| DEBT-FID1-027 | PARKED_BLOCKER | the curb writer plus persist/pose contract is proven **and** the cold-boot route requires `m0001i → m0002i` | BLOCKER for Day1 Candidate if curb is on the critical path |
| DEBT-FID1-028 | PARKED_BLOCKER | a Day-2 / day-boundary script is on the route | BLOCKER for that boundary until the reset writer is proven or proven absent |
| DEBT-FID1-029 | UNKNOWN_LABEL | a later first-play arm issues `0x41` | named bit or BLOCKER if that arm is required |
| DEBT-FID1-030 | UNKNOWN_LABEL | those actors become interactive or a later arm requires a rendered name | named actors |
| DEBT-FID1-031 | UNKNOWN_LABEL | **PE-RD7-A** or later progression walks the `0x04` thread | named body or BLOCKER if that thread is required |
| DEBT-FID1-032 | PARKED_BLOCKER | BTL1+ implements the handoff and a later battle rung owns ATB/commands for the proven m0005i encounter | BLOCKER for Day1 Candidate (SYS0 §7) |
| DEBT-FID1-033 | NONBLOCKING_FIDELITY | TXT1 / the m0372i reel renderer closes on `0xF9` after `FB 07` and treats `OP_0x02` as script-only | BLOCKER if a later reel claims frame-exact window state |

## Special ruling already on the books

Mailbox DEBT-FID1-004 is the only row whose flip is already
written in SYS0:

```text
PROVEN_OUTCOME / NONBLOCKING_FIDELITY  ->  BLOCKER
```

the moment any of these is true:

- battle init or battle return reads or writes the same
  task/mailbox tables
- save/load serializes task, actor, or mailbox state
- a later first-play script depends on task re-arm, `task+0x08`
  flags, or sender serial rather than the payload byte

BTL0 makes the first of those concrete: m0004i mailbox 3/4 is
the first Day 1 combat trigger.

## Retired (do not promote; already replaced)

| former shim | replaced by |
|---|---|
| 8-word persist vector | PST0 512-word `D_800A77F0`; UE0 `PersistState::kWords == 512` |
| `persist[0x4A]==0` treated as 9 | RD6-A / PST0 exact writes 9 / 0x12 / 0x18 |
| SYS0-008 "text IDs only" as the whole text gap | TXT0 letter map + window; residual is atlas + `0x4B` |
| SYS0-011 m0377i dest contract incomplete | RD7-R dest contract |
| RD5-C host-tick auto-close | RD5-C2 `0x100` edge; RD5-F9 proves m0372i was not over-gated |
