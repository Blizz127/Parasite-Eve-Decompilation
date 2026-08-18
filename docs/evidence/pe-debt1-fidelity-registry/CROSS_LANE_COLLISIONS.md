# PE-DEBT1 — cross-lane collisions

A collision is two (or more) lanes shimming the **same retail
mechanism** in **different** ways. Those rows fail first when the
lanes are asked to share a surface.

This rung does not pick a winner and does not patch any lane.

## C1 — mailbox / task (DEBT-FID1-004) — will fail first

Retail authority (RD5-X `MAILBOX_PROOF.md`):

```text
0x1C  func_80017764
  -> func_800653B8   append 12-byte record at D_800A3180 + count*12
       +0 u16 type  +2 u8 id  +3 u8 payload
       +4 u32 extra  +8 u32 sender serial (actor+0x24)
  -> func_80065400   match actor+0x0C/+0x0D
  -> func_80012700   new task at actor+0x19C
       task+0x00 = entry PC
       task+0x08 |= 4
       task+0x0C = sender serial
       task+0x14 = payload byte
```

Three live treatments:

| lane | commit / artifact | what it does | what it does not do |
|---|---|---|---|
| Python RD5-X / RD5-C / RD6-A | `29c095a` / `1e7f0df` / `65446c8`; `pe_rd5c_cutscene.py` `_drain_type2_mailbox`; `pe_rd6a_destination.py` `M0378iStartup` | payload-byte outcomes: 1=noop, 2=clip `0x0A`, `0xFF`=restore+idle, 3=hop | no `D_800A3180` records; no `task+0x08` flags; no sender serial; no re-arm PCs; `+0x05B8` tail abstracted |
| this checkout native | `func_8006536C_port.c` | boot-clears 28×12-byte `D_800A3180` and index byte `0x8009CDB4` | `func_800653B8` / `65400` / `12700` untranslated; table stays empty |
| UE0 native VM | PE-MBX2 `pe_mailbox.cpp` / `pe_vm.cpp` | 28×12 `D_800A3180` queue; `0x1C` append; `65400` drain before `35558`; `0x1F` reads `task+0x14`; no ACK | re-arm PCs / `+0x05B8` tail not claimed; 29th write / stale bytes / hop persist left unsafe |

They agree on **visible prefix outcomes** (clip 10, restore,
token) and disagree on **state**. SYS0
`UE_NATIVE_PARITY_POLICY.md` already names a forbidden
representation-only excuse: "mailbox omitted because the native
task table is empty."

Why this fails first: BTL0 proved the first Day 1 fight is
**m0004i mailbox 3/4 → m0005i**, not the RD6-A north volume.
The moment BTL1 (or UE0 past the lobby, or save/load) shares
`D_800A3180`, the Python payload machine and the empty native
table cannot produce the same task records.

RD7-R mailbox `0xFF` is proven end-to-end on paper and is **not**
a clone of m0378i PCs (`+0x01CC`/`+0x0274`). An outcome-faithful
clone of the m0378i restore would be a fourth treatment.

## C2 — `0x85` / `0x9C` fade (DEBT-FID1-001)

Retail: `0x85` = `func_80018EB4` timed fade; `0x9C` =
`func_80019410` fade gate.

| lane | treatment |
|---|---|
| RD3-A contract | hard-cut **allowed** and must be labeled |
| UE0 `pe_vm.cpp` | hard-cut: both cases advance PC immediately |
| RD6-A `NorthVolumeHop` | `0x85 0x1E` starts mode-2 for 30 ticks; `0x9C` waits `(D_800BCFEE & 3) < 2` |

Same opcodes. Different time. Harmless on UE0's current
m0002i→m0003i hop (RD3-A allowed it). It becomes a collision
when UE0 implements the m0004i north hop or any script that
publishes persist/token **after** the `0x9C` gate. The RD6-A
trace counts those 30 ticks; a hard-cut UE0 hop would shrink
the row count.

Related but not merged: RD7-R allows source-arm `0x85`/`0x9C`
as a hard hold and dest `0x86` mode-6 30 without visual
interpolation. That is the same family, still labeled.

## C3 — persist bank width (DEBT-FID1-017)

Retail (PST0): `D_800A77F0`, 512 words, `0x800` bytes.

| lane | width | covers Day-1 m0005i slots 0x50/0x54/0x64? |
|---|---|---|
| Python RD6-A fixtures | `[0] * 0x4B` (75 words, indices 0..0x4A) | no |
| UE0 `PersistState` | 512 words; index `0x1FF` addressable | yes |
| this checkout `func_80034F10` | 512-word zero | yes |

UE0 and this checkout **agree** with PST0. Python is the
outlier. No collision on the current prefix (only 0 / 1 /
`0x4A`). Collision becomes live when BTL1 / m0005i writes
`persist[0x50]` / `[0x54]` / `[0x64]`, or when save emits 512
words from a 75-word host vector.

The retired 8-word vector and "`0` means `9`" are **not**
current collisions. They are named in PST0
`FIDELITY_RISKS.md` as the previous shape.

## Merged, not a live collision — view 0 (DEBT-FID1-002)

UE0 warning "lobby camera stays view 0" and RD7-R unknown
"whether the engine auto-applies view 0 without `0x82`" are
the **same open question**. One registry row.

They are not two competing shims of a proven rule. They are
one unproven rule plus one stand-in (always view 0) plus one
negative (m0377i does not apply authored view 0).

Future collision if UE0 loads m0377i with always-view-0:
RD7-R contract says "Must not call `0x82` / apply 52-byte
view 0 on this arrival."

The 16-byte init-table **layout** (DEBT-FID1-003) stays
separate. OR-2 onto records 1 and 7 is an outcome, not the
apply-without-`0x82` question.

## Checked, not a collision

| pair | why it is not a collision |
|---|---|
| native `func_80034F10` persist zero vs PST0 bank | same 512-word `D_800A77F0` |
| native `D_8009D28C` setters 0/5/6/8 vs BTL0 | matching leaves are exact; the missing 7 is a parked hole, not a second machine |
| TXT0 letter map vs SYS0 "IDs only" | TXT0 superseded the encoding gap; residual is atlas + `0x4B` |
| RD5-C auto-close vs RD5-C2 `0x100` edge | RD5-C2 replaced the blocker; RD5-F9 proves m0372i was not over-gated |
| RD7-R mailbox `0xFF` vs RD6-A mailbox `0xFF` | same outcome class; different PCs (RD7-R forbids cloning) — one debt row, not two shims |
| Phase 6E GPU guest writes vs persist | different addresses; PST0 already recorded this negative |
| CAM-B / VIS-B / VIS-C freeze vs UE0 view 0 | freeze is `PROVEN_EXACT` for the PT1 stack; view 0 is the documented lobby stand-in under that freeze, not a second camera campaign |

## Count

```text
cross_lane_collisions=3
merged_duplicates=1
highest_risk_collision=C1_mailbox
```
