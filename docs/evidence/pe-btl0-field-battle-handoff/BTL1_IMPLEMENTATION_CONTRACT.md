# BTL1 implementation contract

BTL1 implements the **handoff**, not the battle engine.

```text
btl1_ready=YES
```

## Must

1. Accept m0004i mailbox 3 and 4 after the first-play reel and run
   the existing `0x31 0xA80002C8` hop to m0005i (same dest path as
   other `0x31` hops).
2. On m0005i `persist[1]==4` entrance, execute module 2 through
   `0x1A` / `0x6F` / `0x5A` / `0x70` / `0xB7` using the retail
   handlers’ **observed stores** (slot table + tagged fields).
3. Implement opcode `0x89` as `D_8009D28C = 6` (call or duplicate
   the matching leaf).
4. Implement `0x94` as a read of that word.
5. Advance a battle tick far enough to **consume** 6 → 0 and set
   the `gp+0x10C` edge, matching `0x800299F4`.
6. Emit the TRACE_CONTRACT rows through `mode6_consumed`.
7. Leave persist bytes unchanged across `0x89`.
8. Keep PT1 visual freeze; do not invent an arena camera.

## Must not

- Decode or implement ATB, command menus, damage, PE, or AI.
- Auto-resolve HP to skip the fight and still claim BTL1.
- Name enemies.
- Treat `0x1A` as battle start.
- Zero persist on entry.
- Implement `m0012i+` fights.
- Implement the second/third m0005i `0x89` unless the first
  handshake already matches.
- Guess the writer of mode 7; the wait loop may stop at
  `mode6_consumed` in BTL1.

## Done when

```text
m0004i mailbox 3 or 4 -> m0005i
0x1A variant recorded
D_800A5D58 inUse==1 after 0x6F
D_8009D28C  0 -> 6 -> 0  on the consume tick
persist_hash unchanged on the 0x89 row
trace 3/3
no ATB
```

## Next (not BTL1)

BTL2: active loop, mode 7 store, first command, HP fields.
Boss identity remains RESEARCH_REQUIRED.
