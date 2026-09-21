# Battle reward progression: original setup and confirmation

Update 2026-09-13: the loot renderer/transfer graph is now translated, and the
shared item-base state is corrected. [Current loot evidence](BATTLE_LOOT_SCREEN.md).
The frame-19,301 stop below is historical; level animation remains incomplete.

The previous native reward handler discarded the BP and item-list arguments,
allocated no real reward window, and closed without storing experience. The
62,000-frame supply replay therefore retained zero experience even after its
winning battles. That historical result is not the current route regression
result and cannot establish reward or progression fidelity.

`battle_reward_port.c` replaces those cuts with the original setup, score
animation, confirmation, level/stat commit, ability unlock notifications, and
loot-window construction. The original level and loot callbacks are installed;
remaining unported callbacks still stop through the existing menu dispatcher.
The level animation and the item transfer/confirmation graphs are outstanding.
`func_8002B0E8` also retains separately documented deferred calls; this change
does not claim the entire battle teardown graph is complete.

The normal cold-boot input replay now stops at frame **19,301**, m0013i,
`A80011C8`, story `40`, at missing drawing callback **8004FD68**. It has committed
**2 experience**, retains level byte 0 / maximum HP 45, and has **one pending
reward item**. The real id12/id13/id14 command, loot, and inventory windows are
present. This is an explicit missing implementation exposed by restoring the
reward path, rather than a successful full route.

Local evidence: `/tmp/pe-reward-captured-connected.log` and
`/tmp/pe-reward-connected.bin`, capture SHA-256
`c5c44cfcb0c96e9ed03064da664733587927fdacc6c9cf8c2f68665dce674da6`.
The capture was produced read-only; no captured or synthetic state advances
any connected replay.

## Differential checks

`pe_battle_reward_oracle.py --compare-native --build-dir /tmp/pe-day2-release
--write-header` passes **240 cases** spanning 19 entries and **2,500 executed
original instruction PCs**. It checks original instruction and delay-slot
bytes, return values where meaningful, every native RAM byte below `1FE000`,
and coverage of original writes by the generated fixture ranges. The original
execution stack is excluded. Calls execute the retail code without patched
callee returns. Fixtures deliberately vary experience thresholds, signed
comparison and wrapping cases, BP saturation, pending items, score snapping,
level commits, learned abilities, free inventory runs, and cursor bounds.
`retail_battle_reward_cases.h` and `test_battle_reward.h` carry these cases into
the native suite. Older mode-2 tests now supply real window/stat resources and
assert a real id20 window; the score-confirmation test also asserts XP storage.

The 17 newly complete/replaced function bodies are listed below. Ends are
exclusive. The already translated `4BE4C` and `4BF08` also have full graph cases.
These are native translations, not new matching `src/` units. The rebuilt PSX
EXE remains SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, identical to retail.

| Original span | Words | Original byte SHA-256 |
|---|---:|---|
| `8004B70C..8004B90C` | 128 | `07188e95b2f088c552d15ba8e5728c8f37233f988f8f4b6acaf4d4344a37caf9` |
| `8004B90C..8004B970` | 25 | `ff9f7fb6ea3c47f843676a6b851671d5a17580cf2286910378cdf7cfd90c8b1c` |
| `8004B970..8004BB80` | 132 | `2a3cedb91ff7a982e30565b69c9766c6cd7f029c659c8a2f557f8ae6a385fca1` |
| `8004BB80..8004BC80` | 64 | `47f2a9ad27b7d3d93edcabc3cf5d8a87862eae11d4fe94ea195442387a89e34e` |
| `8004BC80..8004BCB4` | 13 | `5a669c49086b2d15fb79d16af408553cfcbe32da86f0af9c4ef183a5a0f3fe00` |
| `8004BCB4..8004BCE8` | 13 | `aabb9892978752041fafcfc0014567695e28d52685c02ed4de14f1c21bfb6987` |
| `8004C1E0..8004C34C` | 91 | `9b688aa2ae84c1a4237b069d03e4713200b2fd4d7cc2d4d87d73778f9c7ba781` |
| `8004C4B4..8004C520` | 27 | `62f758aae62ecabe7ae457d8287eddfed037624fc1670cbbac3165e3ff4ed0b6` |
| `8004C520..8004C594` | 29 | `8e96454267340e43dcd99500716569c2d1585822195587672c341f1b6334d589` |
| `80051DF8..80051E48` | 20 | `664c5eb251716a3d3c9fe44a67a5659e01add61ca5454031bcdfb338a140ae28` |
| `80052764..80052790` | 11 | `9cf87bd0deb8aca2082cbfd6c3259c965546a47f22faba9d6f0adf207b12b489` |
| `8005382C..80053968` | 79 | `c9d79218d158c686643796e7641610c0660987624d9e6e5eb192617e2f91379c` |
| `80055668..800556E8` | 32 | `018521c58d79908fcc6b0667bbbaedbf3ea41ccf72689c087f613fb6ce9a5c4c` |
| `80057ECC..80057ED8` | 3 | `5132238932a54443f0417873afcec7cb36011eddf59a1cfd1e08987db9d5073a` |
| `8005B8A8..8005B91C` | 29 | `4500589ab365e1b16113ad2f7f5deac24ad1223d1db870c0b17a0c6f9afdbae8` |
| `80063D78..80063E0C` | 37 | `c10bc89f3e065fdd1b8e3dc1fa9251435146bb7bb040f1843fdb5745bf036def` |
| `80048654..80048838` | 121 | `5a872ae4a4259e63aaa03aa2809c0ba92ddb88c663cb9e4be7bae0e2a6b3ff1d` |

Release and Debug builds succeed. After the updated fixtures were rebuilt,
all **10 CTest checks excluding the known failing route** pass in 62.61 seconds;
**1,389/1,389 native tests** pass (60.25 seconds). The route was run separately
and fails at the documented callback. Logs: `/tmp/pe-reward-ctest.log`,
`/tmp/pe-reward-tests-build.log`, `/tmp/pe-reward-debug-build.log`, and
`/tmp/pe-reward-oracle.log`. Python compilation and `git diff --check` pass.

## Remaining work

Translate the reached `4FD68` / `50BE8` loot renderer, its command renderer and
`48838`/`58030` item handling graph; compare them from the captured context.
Complete `4BF40` and its stat-bar drawing dependencies before claiming level
animation coverage. Then rerun controller-only progression and revise the old
fixed route from actual outcomes. Full Day 2 and whole-route fidelity remain
unproved.
