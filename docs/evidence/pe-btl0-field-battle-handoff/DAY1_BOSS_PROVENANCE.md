# DAY1_BOSS_PROVENANCE

Bounded search only. No boss battle decode. No walkthrough names.

## Method

Same mechanism as the first fight: opcode `0x89` → `D_8009D28C = 6`.

Census of `0x89` on table indices 0–20 (Carnegie-adjacent maps):

| Scene | Sites |
|---|---|
| m0005i | `mod6+0x350C`, `+0x3728`, `+0x414C` |
| m0012i | `mod6+0x5EB0` |
| m0013i | `mod3+0x2324` |
| m0014i | `mod8+0x4B38` |
| m0016i | `mod7+0x46B4` |
| m0001i–m0004i, m0009i, m0367i | none |

m0005i therefore contains **three** mode-6 requests. The first is
the ordinary opening fight. The second and third are later arms on
the same module (different mailbox / persist tests, including
`persist[0x0A]` and `persist[0x12]`). None of those arms has a
proven “boss” formation id.

Later maps (`m0012i`…) are not on the SYS0 first-play prefix and
are not claimed as Day 1.

## Result

```text
day1_boss_identity_status     = RESEARCH_REQUIRED
day1_boss_formation_status    = RESEARCH_REQUIRED
source_field                  = unproven
trigger                       = unproven
```

SYS0 blocker `day1_boss_identity_absent` stays open.

A later rung may promote one of the extra m0005i `0x89` sites, or a
later map, only with persist-gated first-play evidence and numeric
formation IDs. Do not assign a character name.
