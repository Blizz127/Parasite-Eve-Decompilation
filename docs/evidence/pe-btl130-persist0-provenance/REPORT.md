# PE-BTL130 REPORT — persist[0]&4 provenance

```text
PE-BTL130 PROVEN — persist[0] bit 4 (0x4) SET BY m0360i ONLY
```

Evidence-only initially. Matching/native implementation follows.

## A. The missing writer: m0360i

Comprehensive scan of **all 438 field table entries** (414 with valid scripts)
found exactly **one scene** that sets persist[0] bit 4:

| Scene | Table index | Module | PC | Operation |
|---|---:|---:|---:|---|
| m0360i | 359 | 2 | +0x06CC | `cond[1] = persist[0] \| 0x4` |
| m0360i | 359 | 2 | +0x06E4 | `persist[0] = cond[1]` |

Effect: `persist[0] |= 0x4` — sets bit 4 while preserving all other bits.

No other scene in the entire game (all 438 field table entries) OR's bit 4
into persist[0]. No EXE direct store writes bit 4. The field-script binder
VM is the only mechanism.

## B. Why the PST0 scanner missed it

The original `pe_pst0_scan.py` scanned 8 scenes (the Day-1 current route
plus Day-1 extras): m0001i, m0002i, m0003i, m0004i, m0005i, m0372i,
m0377i, m0378i. m0360i is table_index 359, not in that set.

## C. m0360i is NOT on the first-play route

The Day-1 current route (PST0 CURRENT_ROUTE_TIMELINE.csv):

```
m0002i → m0003i → m0372i → m0004i → m0378i → m0377i
```

persist[0] after this walk: **0** (confirmed by PST0 route simulation).

m0360i is a later-game scene. The route simulation never reaches it on
first play. persist[0x4A] maxes at 0x18 on the first-play route; m0360i
requires higher thresholds.

## D. First-play m0005i: persist[0]&4 is CLEAR

Since m0360i is not visited on the first play, and no other mechanism sets
bit 4, persist[0] = 0 when m0005i is first entered.

The m0005i +0x091C check (`persist[0] & 0x4`) **always branches**
(skip-if-false → +0x3B4C) on first visit. The 0x81 payload chain does
NOT fire. The type-6 MAIN task waits at +0x190 while scratch[0]&4 is
clear — permanently on first visit.

## E. First visit: Actress cutscene, not Eve battle

On first visit to m0005i:

```text
dest-ready 1266C zeros scratch
125E0 type 1 then type 6
type-6 +0x180 0x14 → +0x19C=+0xD08; wait scratch[0]&4 (never released)
type-1 0x08 types 3,0,5,2,4 (persist[0x4A]<40)
type-1 0x1C(0,0,0xFF); park
type-0 mailbox 0xFF, persist!=39, park
type-2 0x20 park (no body — 0x6F never fires without 0x7F)
type-3 0x77 miss-loop
type-5 +0x1A0=+0xF8
no 0x1C to type 6 with 0x81
```

The first visit is a scripted sequence (Actress dialogue), NOT the Eve
boss battle. The battle chain (0x81 → 0x55 → 0x65 → 0x7F/0x70 → 0x6F)
requires m0360i to have already set persist[0]&4.

## F. Revisit: the authentic Eve battle

After visiting m0360i (Day 2+), persist[0] has bit 4 set. Re-entering
m0005i:

```text
persist[0]&4 is NOW SET
type-4 0x77 volume hit → mails type-0 payload 0x0D
persist[0]&4 gate at +0x091C: PASS (bit is set)
type-0 +0xCAC → 0x1C(6,0,0x81)
type-6 0x81 arm: +0xFC8 0x55 → 0x89 → 0x1C(0,0,0x65) → park
type-0 0x65 arm: 0x1C(type2,0x7F) → 0x1C(type6,0x70) → scratch[0]|=0x10
type-2 0x7F → +0x804 0x6F → 2F7D8 → D20C body creation
type-6 0x70 → wait scratch[0]&0x10 → 0xAE → +0x1850 → scratch[0]|=4
MAIN type-6 wait released → 0x89 → 0x1C(2,0,0x7D) → handshake
```

This is the authentic Eve battle chain.

## G. EXE census: 5 persist base sites

All confirmed by binary scan:

| # | ROM addr | Function | Role |
|---|---:|---|---|
| 1 | 0x0079CC | binder mode 2 | VM dispatch (indirect) |
| 2 | 0x025718 | func_80034F10 | new-game zero (512 words) |
| 3 | 0x030008 | func_8003F800 | save (read-only on persist) |
| 4 | 0x0303E0 | func_8003FBD8 | load (restores from save) |
| 5 | 0x0439CC | func_80053128 | inventory sync (read-only) |

No EXE direct store writes persist[0] bit 4. All OR/AND operations go
through the binder VM ALU dispatch (handlers at ROM 0x0030D4/0x0030F4).

## H. persist[0] bit census (all 438 scenes)

| Bit | Mask | Set by | Cleared by | Tested by |
|---|---:|---|---|---|
| 0 | 0x1 | m0290i, m0431i | indirect | 30+ scenes |
| 1 | 0x2 | 18 scenes | 11 scenes | 20+ scenes |
| 2 | 0x4 | **m0360i ONLY** | never | 31 scenes |
| 3+ | 0x8+ | unknown | unknown | unknown |

Bit 4 is write-once: set by m0360i, never cleared by any script.

## I. Type-4 0x0D prerequisite

BTL129 established that the 0x81 chain also needs type-4 payload 0x0D.
Type-4 0x0D requires:

1. type-4 `0x77` volume hit (player enters retail volume)
2. `persist[0x4A] ∈ [17, 40)` — the m0004i write of 0x18 satisfies this

Both are naturally satisfied on a revisit when the full route has been
played through m0004i.

## J. persist[0x4A] arrival value

The PST0 route timeline shows m0004i module 0 writes:

```
persist[0x4A] = 0x18
```

at +0x0544 (after checking `persist[0x4A] < 0x18`).

0x18 = 24, which is ≥ 17 (type-4 prerequisite) and < 40 (type-1 spawn
gate). This value survives into m0005i.

## K. Type-4 0x77 volume (m0005i module 4)

Module 4 of m0005i contains the type-4 proximity volume:

```text
+0x2988: 0x77 volume rect (4 vertices, args 8-10 = [0x0, 0x2, 0x4])
+0x29BC: local[4]==1 → skip if false
+0x29E4: persist[0x4A] < 0x28 (40) → spawn gate
+0x2A0C: persist[0x4A] >= 0x11 (17) → send gate
+0x2A34: persist[0x4A] = 0x28
+0x2A44: 0x1C(0, 0, 0x0D) — send payload 0x0D to type 0
```

Both gates pass when `persist[0x4A] = 0x18` (from m0004i):
- 0x18 < 0x28 → TRUE (spawn gate passes)
- 0x18 >= 0x11 → TRUE (send gate passes)

The volume is a retail-authored rectangular region. The player must
physically walk Aya into this volume for the 0x77 hit to register.
Do not auto-trigger the volume.

## L. Full chain verification (revisit m0005i)

With `persist[0]&4` set by m0360i and `persist[0x4A]=0x18` from m0004i:

```text
1. Player enters type-4 0x77 volume (module 4 +0x2988)
2. +0x2A44: 0x1C(0, 0, 0x0D) — type-4 mails type-0 payload 0x0D
3. Module 0 +0x0738: payload==0x0D → branch to +0x664
4. +0x091C: persist[0] & 4 — PASS (m0360i set it)
5. +0x0CD0: 0x1C(6, 0, 0x81) — type-0 sends 0x81 to type-6
6. Type-6 0x81 arm: +0xFC8 0x55 → 0x89 → 0x1C(0,0,0x65)
7. Type-0 0x65 arm: +0x114C 0x1C(2,0,0x7F) + +0x1160 0x1C(6,0,0x70) + scratch[0]|=0x10
8. Type-2 0x7F → +0x804 0x6F → 2F7D8 → D20C body creation
9. Type-6 0x70 → wait scratch[0]&0x10 → 0xAE → scratch[0]|=4
10. MAIN type-6 wait released → 0x89 → 0x1C(2,0,0x7D) → handshake
```

Every link in the chain is now script-verified. No values planted.

## M. Status

```text
persist_0_bit4_provenance=PROVEN
persist_0_bit4_writer=m0360i module 2 +0x06CC +0x06E4
persist_0_bit4_operation=persist[0] |= 0x4
persist_0_bit4_first_play=NEVER_SET
persist_0_bit4_revisit=SET_AFTER_M0360I
eve_battle_gating=FIRST_VISIT_ACTRESS_ONLY
eve_battle_requires=m0360i_visited (Day 2+)
type4_0x0D_volume=m0005i module 4 +0x2988 (authored rect)
type4_0x0D_requires=persist[0x4A] >= 17 AND < 40 AND volume hit
type4_0x0D_arrival_value=persist[0x4A] = 0x18 (from m0004i)
type4_0x0D_gates_both_pass=YES (0x18 in [17,40))
body_creation_chain=PROVEN_SCRIPT_VERIFIED
full_chain=persist[0]&4 → 0x0D → 0x81 → 0x55 → 0x65 → 0x7F/0x70 → 0x6F → D20C
```

## Files

```text
docs/evidence/pe-btl130-persist0-provenance/REPORT.md
tools/research/pe_pst0_all_scene_scan.py
tools/research/pe_btl130_persist0_writer_scan.py
docs/evidence/pe-pst0-persist-provenance/persist0_all_scenes.json
docs/evidence/pe-pst0-persist-provenance/PERSIST0_COMPREHENSIVE_REPORT.md
```
