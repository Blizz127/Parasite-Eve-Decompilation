# PE-BTL132 REPORT — m0360i Day 2+ routing hub

```text
PE-BTL132 ANALYZED — m0360i is the Day 2+ game routing hub
```

## A. m0360i overview

8 modules, 1362 total commands. Table index 359.

## B. Module structure

| Module | Commands | Role |
|---:|---:|---|
| 0 | 22 | Entry checks |
| 1 | 20 | Actor creation (types 0-7) + dest 0xA8081448 |
| 2 | 76 | **persist[0]|=4** + text 0xB6 + payloads 0xFE/0xFF |
| 3 | 265 | **Routing hub** — 31 dest tokens, 25 persist[0x4A] writes |
| 4 | 97 | Secondary routing — 10 dest tokens, 10 persist[0x4A] writes |
| 5 | 407 | Dialogue/events — 8 payload sends, no dest tokens |
| 6 | 191 | More dialogue/events — 2 payload sends |
| 7 | 72 | Flag setting — persist[7] |= 0x2/0x4/0x8/0x100/0x200/0x400/0x800 |

## C. Module 2 — the bit-4 writer

```text
+0x032C: persist[0x4A] == 0 → skip
+0x0354: scratch[0x12] == 0 → skip
+0x06CC: persist[0] |= 0x4   ← THE UNIQUE BIT-4 WRITER
+0x06E4: persist[0] = cond
+0x0728: 0x0D (open text message 0xB6)
+0x0774: persist[1] = 0x3E6
+0x078C: 0x1C(0, 0, 0xFF)
```

Gates: persist[0x4A] != 0 AND scratch[0x12] != 0 AND persist[0x4A] != 4.

## D. Module 3 — Day 2+ routing (31 dest tokens)

Each route follows the pattern: persist[0x4A] = VALUE → 0x85 wait → 0x31 hop.

| persist[0x4A] | Dest token | Notes |
|---:|---|---|
| 0x00 | 0xA80830C8 | First route |
| 0x09 | 0xA80010C8 | |
| 0x68 | 0xA8002348 | |
| 0x70 | 0xA80654C8 | |
| 0x88 | 0xA8004148 | |
| 0xC0 | 0xA80290C8 | |
| 0xD8 | 0xA8004348 | |
| 0xE4 | 0xA8067248 | |
| 0x120 | 0xA80040C8 | |
| 0x128 | 0xA8009048 | |
| 0x130 | 0xA80090C8 | |
| 0x160 | 0xA80290C8 | |
| 0x180 | 0xA80201C8 | |
| 0x1C8 | 0xA80222C8 | |
| 0x210 | 0xA8004148 | |
| 0x220 | 0xA80260C8 | |
| 0x238 | 0xA8046048 | |
| 0x258 | 0xA8029148 | |
| 0x2A4 | 0xA80454C8 | |
| 0x2A8 | 0xA80470C8 | |
| 0x2B0 | 0xA8047148 | |
| 0x2CA | 0xA80471C8 | |
| 0x2DA | 0xA8082448 | |
| 0x2E2 | 0xA8048448 | |
| 0x2E2 | 0xA80671C8 | (duplicate value, different dest) |

## E. Module 4 — secondary routing (10 dest tokens)

| persist[0x4A] | Dest token |
|---:|---|
| 0x78 | 0xA8003348 |
| 0xB8 | 0xA8003448 |
| 0xD0 | 0xA8004048 |
| 0xE0 | 0xA80034C8 |
| 0x148 | 0xA8003448 |
| 0x178 | 0xA80033C8 |
| 0x1C0 | 0xA80034C8 |
| 0x208 | 0xA80032C8 |
| 0x298 | 0xA80033C8 |
| 0x2A2 | 0xA80034C8 |

## F. Module 7 — flag setting

Sets bits in persist[7]:
- 0x2, 0x4, 0x8, 0x100, 0x200, 0x400, 0x800

Each bit is OR'd individually (7 separate OR operations).

## G. Dest token format decoded

```text
scene_table_index = ((token & 0xFFFF) - 0x248) / 0x80 + 3
```

Verified against all known tokens:
- 0xA80000C8 → m0001i (index 0) ✓
- 0xA8000148 → m0002i (index 1) ✓
- 0xA80001C8 → m0003i (index 2) ✓
- 0xA8000248 → m0004i (index 3) ✓
- 0xA80002C8 → m0005i (index 4) ✓
- 0xA8001248 → m0036i (index 35) ✓

## H. Module 3 routing decoded

| persist[0x4A] | Token | Scene |
|---:|---|---|
| 0x00 | 0xA80830C8 | m0096i |
| 0x09 | 0xA80010C8 | m0032i |
| 0x68 | 0xA8002348 | m0069i |
| 0x70 | 0xA80654C8 | m0168i |
| 0x88 | 0xA8004148 | m0129i |
| 0xC0 | 0xA80290C8 | m0288i |
| 0xD8 | 0xA8004348 | m0133i |
| 0xE4 | 0xA8067248 | m0227i |
| 0x120 | 0xA80040C8 | m0128i |
| 0x128 | 0xA8009048 | m0287i |
| 0x130 | 0xA80090C8 | m0288i |
| 0x160 | 0xA80290C8 | m0288i |
| 0x180 | 0xA80201C8 | m0002i |
| 0x1C8 | 0xA80222C8 | m0068i |
| 0x210 | 0xA8004148 | m0129i |
| 0x220 | 0xA80260C8 | m0192i |
| 0x238 | 0xA8046048 | m0191i |
| 0x258 | 0xA8029148 | m0289i |
| 0x2A4 | 0xA80454C8 | m0168i |
| 0x2A8 | 0xA80470C8 | m0224i |
| 0x2B0 | 0xA8047148 | m0225i |
| 0x2CA | 0xA80471C8 | m0226i |
| 0x2DA | 0xA8082448 | m0071i |
| 0x2E2 | 0xA8048448 | m0263i |
| 0x2E2 | 0xA80671C8 | m0226i |

## I. Module 4 routing decoded

| persist[0x4A] | Token | Scene |
|---:|---|---|
| 0x78 | 0xA8003348 | m0037i |
| 0xB8 | 0xA8003448 | m0039i |
| 0xD0 | 0xA8004048 | m0128i |
| 0xE0 | 0xA80034C8 | m0039i |
| 0x148 | 0xA8003448 | m0039i |
| 0x178 | 0xA80033C8 | m0038i |
| 0x1C0 | 0xA80034C8 | m0039i |
| 0x208 | 0xA80032C8 | m0036i |
| 0x298 | 0xA80033C8 | m0038i |
| 0x2A2 | 0xA80034C8 | m0039i |

## J. Module 1 entry dest

| Token | Scene |
|---|---|
| 0xA8081448 | m0039i |

## K. Key insight

m0360i routes the player through 25+ unique scenes across the Day 2+
game. The routing starts at m0096i and progresses through increasingly
high-numbered scenes. The route returns to m0002i (Carnegie Hall) at
persist[0x4A]=0x180, suggesting a mid-game return to a familiar location.

## L. Status

```text
m0360i_role=DAY2_ROUTING_HUB
m0360i_bit4_writer=MODULE_2_+0x06CC
m0360i_dest_tokens=31+10
m0360i_persist_0x4A_max=0x2E2
m0360i_persist_7_flags=0x2+0x4+0x8+0x100+0x200+0x400+0x800
```

## Files

```text
docs/evidence/pe-btl132-m0360i-hub/REPORT.md
```
