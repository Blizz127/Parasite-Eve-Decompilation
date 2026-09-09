# Station return and following world-map scene

Stage106 verifies the closed return progression following M0037I's D8
endpoint. Reproduce with `pc_port/tools/pe_day2_station_return.py`; ignored
results are `local/live/day2-station-return-106.json`. Story means persist74;
persist indices are decimal and story/mask values are hexadecimal.

## Verified endpoints

| Original region | Condition and effect | Stop boundary |
| --- | --- | --- |
| M0046I module1,801AB68C | D8 becomesDA; all other input stories remain unchanged | 801AB6C4, next previous-room comparison |
| M0047I module2,801D7D3C | DA selects the return-scene arm; other stories leave it | 801D7D64 or801D8A28 |
| M0047I module2,801D89E8 | writesE0 after the preceding counter wait | 801D89F8, fade |
| M0039I module2,801CBBDC | E0 selects the scene; other stories reach the later selector | 801CBC04 or801CC9AC |
| M0039I module2,801CBFA8 | writesE4 and prepares map flags | 801CC9A0, M0000I transfer |

The M0047I E0 write is tested from its own instruction. This does not execute
or establish completion of the counter wait or preceding dialogue. Likewise,
the scene gates and final blocks are separate evidence; the asynchronous
middles remain unfinished. The station's D8->DA write alone is not an ending.

## Map selection following E0

Running the original M0046I mask-preparation block at storyE0 gives
`prior | 3E000 | (persist0 & 1 ? 800000 : 0)`. Original M0000I92030 with
selection8 and that state selectsM0039I, stopping before74D28/SDK work.
Selection8's enable bit is mask20000, established by stage104.

The original M0039I final block writesE4, persist1=999 and preserves that
prepared mask. Feeding its resulting state into original92030 selection8
selectsM0374I. Eight compositions cover four prior masks and both persist0
mask1 states. These are endpoint compositions, not actual scene loading,
menu input or live traversal.

## Newly pinned scripts

M0039I SHA256
`5ba3c237d6fc44d079abc1d870c4a1b88d6d51a753ba417eb0eb6dcdc204775b`,
script base801CB1D0. Static writes2A4/E4/1C8 and destinationsM0259I/M0000I/
M0000I demonstrate its shared content. Only its E0 gate and E4 final block
are executed by this audit.

M0374I SHA256
`01b08019e76adebf20aeee364cf042b503fa2e4472885824bde7b8098a2cf33d`,
script base801C4DE4. Static story writesE6/E8/E8 occur at801C5D54/801C605C/
801C6120. Immediate destinations areM0000I at801C6F68 and801C7A58, andM0059I
at801C7B38. These are inventory evidence, not execution or Day2 membership
proof. Its story conditions, battle/scene sequence and exit are next targets.

## Verification limits

The tool executes original handlers with explicit script argument banks and
fixture story/flags. No asynchronous operation is skipped. The tests include
all story0..300 values plus signed extremes for the three gates, one final
E0 assignment, eight E0 map preparations, eight E4 final blocks and sixteen
original selector prefixes. Final counts/completion are in ACTIVE_HANDOFF.
Python and whitespace checks are run. This is research/documentation work;
stage102 remains the latest native build and regression result.

Full Day1/Day2 decompilation and scope classification remain active. Still
follow M0195I->M0230I and other park interiors, optional/shared routes, actual
trigger/message delivery and full scene execution. None of the endpoint
checks establishes a completed Day2 playthrough or terminal transition.
