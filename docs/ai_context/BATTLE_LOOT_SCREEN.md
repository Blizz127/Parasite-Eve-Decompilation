# Battle loot screen and shared item-base state

The subsequent [level animation restoration](BATTLE_LEVEL_ANIMATION.md) now
completes the first sewer victory and restores field control at frame52,111.
The earlier level drawing boundary below is retained as historical evidence.

The original loot screen now renders its commands and pending items, takes all
eligible rewards, supports manual swaps with equipped-item restrictions, and
finalizes weapon/armor records before returning to the field. The translations
in `battle_reward_port.c` cover ten bodies (758 words); `5833C` was already
translated and is now grouped here with its original explicit bank stores.
Same-slot XOR swaps retain the original three writes. Equipment finalization
loads each sixteen-byte group before storing it. Legal physical RAM reads
through a zero record pointer retain guest-RAM aliasing rather than aborting.

The matching `50BE8` C leaf has a void prototype even though the MIPS callback
passes its live `a0` to `57F14`. The native wrapper takes that item index
explicitly. CMake excludes the generated zero-argument provider from this
runtime build; the matching source, generated source, and PSX image are intact.
The old boundary test no longer expects this implemented callback to stop.

## Shared state bug

`52C6C` found the special-item base correctly but stored it in a separate host
`D_8009D03C`. The original reward translation read guest `8009D03C`, which still
contained zero. Thus ordinary drop ID 1 was incorrectly remapped to temporary
ID 519. The existing temporary record had subtype 20, so Take All would not
collect it. `D_8009D03C` is now a guest-RAM lvalue, like the adjacent inventory
bank words, and the duplicate host storage is removed.

Actual cold boot now establishes **base ID 26 (`1A`)**. At frame **19,301**, the
m0013i reward has committed **2 XP** and holds **item ID 1**, rather than 519.
Normal input takes it at frame **19,309**, changing reserve ammunition from
**0 to 6**. No gameplay RAM writes or checkpoint restores advance the replay.

Capture `/tmp/pe-loot-correct-item-connected.bin`, SHA-256
`5433f2986981cb86fb649760ec2ecc48e0ce7c63e0775324d65f9adb4499b296`,
comes from a normal cold boot stopped at frame 19,301. The adjacent log is
`/tmp/pe-loot-correct-item-connected.log`; the pickup transition is recorded in
`/tmp/pe-loot-pilot-connected.log` as `LOOT_CONNECTED_STATE`.

## Original/native comparisons

`pe_battle_loot_oracle.py --compare-native --build-dir /tmp/pe-day2-release
--write-header --capture /tmp/pe-reward-connected.bin` passes **198 cases**
across **13 entries**, plus isolated capture operations. The test executes
original instructions, checks their bytes and delay slots, compares returns
where meaningful and every RAM byte below `1FE000`, and checks generated-range
coverage of original writes. Only the original execution stack is excluded.
There are **4,145 executed instruction PCs** in this run. Fixtures cover both
packet banks, item types, full inventory, same-slot and cross-list swaps,
equipped/protected items, weapon-record pools, command events, whole-window
drawing, event dispatch, and the original `52C6C` item-base writer.

The corrected item-ID capture is also compared separately using
`--capture-only --capture /tmp/pe-loot-correct-item-connected.bin`. Its two loot
renderers, full draw tree, initializer, and Take All/Done sequence all match
native full compared RAM and returns (**3,437 executed instruction PCs**).
Captured execution remains an isolated comparison, never connected progress.
Logs: `/tmp/pe-loot-ram-oracle.log` and
`/tmp/pe-loot-correct-capture-oracle.log`. Generated native regression data:
`retail_battle_loot_cases.h` / `test_battle_loot.h`.

| Original span | Words | Original byte SHA-256 |
|---|---:|---|
| `8004FCF8..8004FD68` | 28 | `1a3e8559a3d5b404a616124192b1885ac0cf9d6b5b372089d5d3dd7c3040c617` |
| `8004FD68..8004FDA4` | 15 | `bcd44b5a6a6d41d8cd97ad9d888b8a769c17476eac68fdfc6e3e03dda96849bd` |
| `80050B94..80050BE8` | 21 | `697a9b4454460851eeaad28033f146b26715c329cce44b151f8daf997099f2f2` |
| `80050BE8..80050C08` | 8 | `6e11024d24eb82b17964737123495c34322c9da48dfc7517c6513dc51d5316d2` |
| `80057F14..80058030` | 71 | `89892947317d1c586254a6b669d73eb30f43275d90cb97d8f4bd721f795bc9e5` |
| `80058030..8005833C` | 195 | `4645800522c00b7895ef9e4135a6ec6a69c31a0660372f292630ca07365c5d11` |
| `8005833C..80058454` | 70 | `3b4b7b47c040f7ae94e42adb327b68e0d587a1c680c1d07006922d843885b503` |
| `80058454..80058670` | 135 | `b47e316e45c358e5c194e73ff5d4ca7dd612d17ef90e1f82f7dd491ca25f66d4` |
| `80058670..800588EC` | 159 | `b810e334551dddb5a8aa807911343c9badbc3dee393a4f64941ad72f576391ad` |
| `80048838..80048918` | 56 | `2db37f5f7e19db7e1e3672291bea1830fec8fcf107d71cb56bdef58607ac8bc5` |

## Connected route limits

The old fixed controller sequence passes the restored loot screen and reaches
the first sewer battle, but loses it. It later stops at a menu boundary after
death/restart, frame 61,593, story 9, room `A8000148`; that later menu is not a
new story frontier. The run observes 45/57 milestones and does not validate the
old supply route. Log/capture: `/tmp/pe-loot-ram-connected.{log,bin}`;
capture SHA-256 `58fd0272dae9de99ed1665378cb006501f563a012c6bb5603d9c241d8a820bf7`.

A first adaptive input pilot also loses, at frame 53,428: it exhausts the pistol
and continues attempting shots. A revised controller now switches to the baton
through the normal weapon menu when ammunition is exhausted. It defeats the
encounter at frame 51,980, then reaches the explicit level draw boundary
`8004BF40` at **52,005**, room m0027i / `A80023C8`, story 68. Aya is alive at
27 HP; XP is 8; display level advances from 1 toward 3, max HP from 45 to 53,
and BP from 0 to 9. Pending loot holds three item-ID-1 entries.
The controller is `/tmp/pe-loot-equip-pilot-route.c`; its normal weapon command
407 selects inventory slot 2 at frame 51,701. Log/capture:
`/tmp/pe-loot-equip-pilot-connected.{log,bin}`, capture SHA-256
`47556529f1ea97f14129e476eac5f1f61d33cb694ea4b2b50191c6eb1bdeb073`.
This is an enemy-defeat/level-screen milestone; field control after rewards
has not yet been restored in this run. Level drawing is the next implementation. Full Day 2, a connected level-up, and whole-route fidelity are still
unproved. The original and rebuilt PSX EXEs retain SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.


Release and Debug builds pass. `ctest --test-dir /tmp/pe-day2-release
--output-on-failure -E route-boot-day2` passes **10/10**, 61.16 seconds,
including **1,390/1,390 native tests**. Logs:
`/tmp/pe-loot-verified-{ctest,lasttest,debug-build}.log`. The excluded fixed
route still fails as documented above; this is not an all-eleven-tests claim.
