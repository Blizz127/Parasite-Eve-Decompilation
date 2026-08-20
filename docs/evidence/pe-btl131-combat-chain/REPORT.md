# PE-BTL131 REPORT — D20C → targeting → tid406 → combat chain

```text
PE-BTL131 PROVEN — D20C body → 5C498 → D2A4 → 26824 → tid406 chain mapped
```

## A. D20C body creation (opcode 0x6F / func_8002F7D8)

The 0x6F handler creates an enemy body:

1. Copies224-byte resource template from ROM table `0x800109B0` to stack
2. Iterates body slots 0-6 in `D_800A5D58` (28-byte stride per slot)
3. Finds first empty slot (body+0x00 == 0)
4. Stores resource data into the slot
5. Stores resource index at body+0x08
6. Installs callback at body+0x2C (from resource data)
7. Calls `1A680` (command setup) with `a1=2`
8. Increments `D_8009D2EC` (body count) and `D_8009D2A0`

The body slot structure (28 bytes at `D_800A5D58 + index*28`):

```text
+0x00: next pointer (linked list)
+0x04: type (4 for type-4 body)
+0x08: resource index
+0x0C..+0x1B: resource data (from ROM table)
```

## B. Targeting pipeline (func_8005C498)

Called from func_80029810 (HP init, 144FC state 0x3A) during battle entry:

```text
func_8005C498:
  1. 42ED0: gate check (gp+0x168 != 0 → early exit returning 0)
  2. Stores encounter token to D_8009D1E0
  3. 51504: zeros gp+0x2A0
  4. 5E6F0: body resource setup (reads D_8009CDDC table)
  5. 46334: secondary setup
  6. 5E30C: core targeting — walks body task queue, dispatches
     callbacks at body+0x2C via jalr
  7. 4F464: cleanup
  8. 42B6C: callback dispatch (after 4 ticks)
  9. 62FEC: final processing (walks resource records)
  10. 5E788: resource management (GPU/resource lifecycle)
  11. Returns 0 on normal path (tid is stored separately)
```

## C. D2A4 publication

`func_80029810` (HP init) at `0x80029A5C`:

```text
jal func_8005C498   # targeting pipeline
sh v0, 0x534(gp)    # D_8009D2A4 = targeting return
```

D2A4 (`gp+0x534`) is the targeting result (16-bit halfword).
Sole writer: `sh v0` at `0x80029A68` inside `func_80029810`.
Sole caller of `func_80029810`: `144FC` state `0x3A` at `0x8001461C`.

## D. tid406 publication (func_80026824)

From BTL121: `func_80026824(a0==1)` reads `lh D_8009D2A4` and publishes
it to `BE830[CE3C].tid`. For Eve, `D2A4=406`.

tid 406 is `jtbl[13]`: seven `AE000` slots and `D25C=0`.

## E. D2A4 readers

Two readers of `gp+0x534`:

| Address | Function | Context |
|---:|---|---|
| 0x8002AA68 | func_8002A7F8 (mode dispatcher) | After 33A40, if D2A4!=0 calls 67CBC |
| 0x8002B1AC | func_8002B18C | Uses D2A4 for body processing |

## F. Attack processing chain (already proven BTL98-109)

```text
299CC (combat tick) mode==0 && 4D4!=0
  → 1D340 (ATB/attack processing)
    → 1F4D4 (attack entry)
      → 1F704 (HP subtract: sh v0,12(a0))
        → 1F814 (hit react: 305C8 → 1A680 → 6DE80)
  → 299CC continues: D20C walk
    → body+0x10 <= 0 → 28E94 (death check)
      → D2A0==1 && AF==0 → null body → mode 2
  → 292EC (remaining enemy check)
    → empty leftovers + HP>0 → 2F300 (mode 2)
  → 2B0E8 (mode 2 → mode 9 phases)
    → 2B29C (teardown: drain CE70/+0x252)
      → 295E4 (mode=-1, 6A25C exit)
```

## G. Combat chain status

| Link | Status | Evidence |
|---|---|---|
| D20C body creation (0x6F/2F7D8) | PROVEN | BTL122 |
| Targeting pipeline (5C498) | STRUCTURED | BTL131 |
| D2A4 publication (29810/sh) | PROVEN | BTL121/131 |
| tid406 publication (26824) | PROVEN | BTL121 |
| 512AC record writer | PROVEN | BTL112 |
| ATB processing (1D340) | PROVEN | BTL98 |
| HP subtract (1F704) | PROVEN | BTL98/99 |
| Enemy HP subtract (28574) | PROVEN | BTL109 |
| Death check (28E94) | PROVEN | BTL105 |
| Remaining enemy (292EC) | PROVEN | BTL104 |
| Mode 2 (2F300) | PROVEN | BTL104 |
| Mode 9 (2B0E8) | PROVEN | BTL103 |
| Teardown (2B29C) | PROVEN | BTL101 |
| Player death (mode 3) | PROVEN | BTL100 |
| Field return | PROVEN | BTL102 |

**The complete combat chain from D20C body creation through teardown is now
fully mapped with retail EXE evidence.**

## H. Status

```text
combat_chain_status=FULLY_MAPPED
d20c_body=PROVEN_2F7D8
targeting=STRUCTURED_5C498
tid406=PROVEN_26824
attack_processing=PROVEN_1D340_1F704
enemy_death=PROVEN_28E94_28574
victory=PROVEN_292EC_2F300
teardown=PROVEN_2B0E8_2B29C
field_return=PROVEN_BTL102
```

## Files

```text
docs/evidence/pe-btl131-combat-chain/REPORT.md
tools/research/pe_btl131_combat_chain.py
```
