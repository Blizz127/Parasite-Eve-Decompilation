# Level-up animation and stat bars

The subsequent [victory-handler restoration](BATTLE_VICTORY_HANDLER.md)
resolves the handler omissions recorded at the end of this report and
documents the corrected connected continuation into M32.

Six complete original bodies (1,022 instruction words) are translated in
`pc_port/game/boot/battle_reward_port.c`. The draw dispatcher now routes
`4BF40`, `4FFF8` and `50D20` to these implementations. No matching sources or
PSX build layout changed.

`4BF40` increments the displayed level on the original 31-tick cadence,
animates max HP and BP, advances stat interpolation, and stops the voice only
when the animation is finished. `437B4` eases increasing fractions and retains
the original four/two/zero timer behavior when wrapping a full stat bar.
`5BA78` computes remaining points and a fractional bar value with signed
threshold comparisons and nullable ordered output stores. Its fraction is
not clamped like `5B91C`; its maximum-level output is 48.

`6062C` renders the remaining points and six independently gated textured
quads, preserving coordinate truncation, UV writes, packet order, dim colors,
and physical-zero RAM writes when packet allocation is exhausted. Its native
helpers are already translated menu operations. The local remaining/fraction
values stay in host locals, with no invented guest scratch location.

## Independent verification

```
python3 pc_port/tools/pe_battle_level_oracle.py --compare-native \
  --build-dir /tmp/pe-day2-release --write-header \
  --capture /tmp/pe-loot-equip-pilot-connected.bin
python3 pc_port/tools/pe_battle_level_oracle.py --capture-only --compare-native \
  --build-dir /tmp/pe-day2-release \
  --capture /tmp/pe-loot-equip-pilot-connected.bin
```

The first run passes **183 original/native cases**, seven entries, and
**2,526 executed instruction PCs**, including the seven isolated captured
operations. Fixtures cover signed threshold extremes, nullable/aliased outputs,
bar reset/hold timers, both packet banks, interpolation phases, all six quad
thresholds, and exhausted allocations. The original instructions and their
delay-slot bytes are checked against the source EXE. All RAM below `1FE000`
is compared with native execution, and generated ranges cover all original
writes; only the original execution stack is excluded.

The subsequent captured-sequence run additionally compares **80 consecutive
full window draws and level confirmation**, totaling **3,488 executed PCs**.
Both executions finish with internal level2 (displayed level3), maxHP53 and
BP9. Each isolated draw resets the packet cursor and coordinate stack as a new
menu drawing context. This is an isolated differential experiment, never an
injected checkpoint in the connected route. Logs:
`/tmp/pe-level-oracle.log`, `/tmp/pe-level-capture-sequence.log`.
Generated regression data: `retail_battle_level_cases.h`, `test_battle_level.h`.

The capture is from the previous normal-input run stopped at frame52,005,
m0027i/A80023C8, HP27, XP8, pending three item1 rewards. SHA-256:
`47556529f1ea97f14129e476eac5f1f61d33cb694ea4b2b50191c6eb1bdeb073`.
See [loot evidence](BATTLE_LOOT_SCREEN.md).

| Original span | Words | Original byte SHA-256 |
|---|---:|---|
| `8005BA78..8005BBE4` | 91 | `7e33d6c9cc91f68f94884b8b0775f534e54895dbe3b072a724996e7ec12026f3` |
| `800437B4..800438C0` | 67 | `90e0d8b7fa6243a46a5b0580ee463c882c1c4cb109f5c3f8f8caf30146436510` |
| `8006062C..80061044` | 646 | `6cb74ab51f442219ac83d64b48ca62f0c4db7c574cdcae978e353c1d65a299e9` |
| `80050D20..80050DC0` | 40 | `d53964f628d696777100b5c5724c5249b7799d488ba1ddf5607fe40e7926d043` |
| `8004FFF8..80050020` | 10 | `6537abe640a742ac8b8cbe183827f277b59e2cdf1bf8df6287f6dd4f3a718bee` |
| `8004BF40..8004C1E0` | 168 | `5ca7d8de33fc3fbceea0da0cd325f5731d4e01f256a498345ff02eaebed38356` |


## Connected replay

The adaptive controller is now reproducible in
`pc_port/tests/route_reward_sewer_pilot.h`, enabled with
`PE_ROUTE_REWARD_PILOT=1`. It preserves the canonical prefix, reads live state,
and emits ordinary movement/menu/attack/heal buttons. It switches from an
empty pistol to the baton through the weapon menu. It never writes gameplay
RAM. The default fixed controller is retained as a historical failing route.

```
PE_ROUTE_REWARD_PILOT=1 PE_ROUTE_FRAMES=57000 \
PE_ROUTE_BATTLE_DUMP_BEGIN=49000 \
PE_ROUTE_RAM_DUMP=/tmp/pe-level-connected.bin \
/tmp/pe-day2-release/pe-route-boot-day2-tests > /tmp/pe-level-connected.log 2>&1
```

The first fresh replay completes the first sewer victory with field control at
**frame52,111**, Aya **27 HP**. At frame52,080, normal Take All has changed
reserve ammunition from 0 to **18**. The run ends at the requested57,000 frame
cap without an unresolved boundary, still in m0027i; the obsolete later input
sequence has opened a menu and has not reached m0028i. It observes46/57 old
milestones. Capture `/tmp/pe-level-connected.bin`, SHA-256
`1a5dd130bd66f894d510590900c106d2077fc41c1d8a91b9c252ffe979d7a5f9`,
retains XP8, internal level2, maxHP53, BP9, reserve18 and mode9.

The pilot now handles both sewer battles and uses the original exit rectangles
for navigation: m27 x[-1000,1000]/z[-1400,-1100], then m28
x[-300,600]/z[300,600]. It closes any open menu with ordinary Circle input.
The next 60,000-frame cold boot uses the same command above with
`PE_ROUTE_FRAMES=60000` and `/tmp/pe-level-forward-connected.{log,bin}`.
It completes the second sewer victory with field control at **53,823**,
Aya **33 HP**, and enters **m0031i / sewer junction at54,520**. It runs to the
60,000-frame limit without an unresolved boundary and observes **50/57** old
milestones. Aya ends at (170,1048,-950), HP33/maxHP53, XP16, internallevel2,
BP9, reserve36, equippedslot2, flags4000, mode9, no menu. The remaining old
milestones are the optional supply-room continuation, not a native stop.
Capture `/tmp/pe-level-forward-connected.bin`, SHA-256
`f31641d79758f7b262f5a28bdff8d6ccca3e5eb01506de890698f8541e73c40e`.
The pilot source SHA-256 is
`bf20be01d5af60b522a19c76cfa2cd39dabf1fcc6c5810e3dcfb4a978284c48a`.

The next route work starts at this corrected junction state. A read-only audit
also confirms two pre-existing omissions in `func_8002B0E8` phase0: original
`703F4` effect cleanup precedes reward creation, then `67CBC` follows it. Both
callees exist natively but remain omitted by the handler. Its phase2 comparison
also narrows the original halfword to a byte. These are not changed or proved
by the level-draw work, and must be addressed before claiming complete victory
handler fidelity. Original dump: `/tmp/pe-victory-original.txt` (2B0E8..2B29C).

Release and Debug builds pass. The new generated regression is included in
`test_native.c`: **1,391/1,391 native tests pass**. `ctest --test-dir
/tmp/pe-day2-release --output-on-failure -E route-boot-day2` passes **10/10**,
70.46 seconds, native test67.92 seconds. Logs:
`/tmp/pe-level-{tests-build,debug-final-build,ctest,lasttest}.log`.
The default old route remains failing and is explicitly excluded from this
result. The source and unchanged candidate EXE both retain SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Full Day 2 and whole-route retail fidelity remain unproved.
