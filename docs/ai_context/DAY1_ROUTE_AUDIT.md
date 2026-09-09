# Day 1: partial original route audit

This inventory records inspected script transfers, not complete reachability,
all optional content, live traversal or Day 1 completion. Original Disc1 is
the authority. `python3 pc_port/tools/pe_day1_route_audit.py` independently
reads the room chunks and verifies35 transfer instructions across15 chunks.
It checks opcode31, one immediate argument, zero second word and the exact
destination token. Output includes chunk LBAs, sizes and SHA256 values.
Executable SHA1: `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

| Source | Inspected destinations | Original script addresses |
| --- | --- | --- |
| M0319I | M0023I (two), M0026I, M0012I | 801A25A4,801A2EDC,801A2A04,801A3308 |
| M0023I | M0319I, M0367I, M0026I | 801D7958,801D7CF8,801D7E6C |
| M0367I (inspected Day1 branch only) | M0319I | 801AA850 |
| M0026I | M0027I | 80193DD4 |
| M0027I | M0026I, M0028I | 80197768, 80197844 |
| M0028I | M0027I, M0029I (two exits), M0031I | 801A5B7C, 801A5D1C, 801A5EBC, 801A6094 |
| M0029I | M0028I (two), M0030I (two) | 8018FA68, 8018FBA4, 8018FD14, 8018FE50 |
| M0030I | M0029I (two) | 80197A64, 80197B68 |
| M0031I | M0032I, M0028I, M0334I | 8019E8A0, 8019EA38, 8019EB40 |
| M0032I | M0033I, M0031I | 8019C888, 8019C950 |
| M0033I | M0034I, M0359I (two) | 801CE404, 801CEB24, 801CEBE8 |
| M0034I | M0359I | 801B5414 |
| M0359I | M0036I, M0042I | 801B8570, 801B8778 |
| M0036I | M0000I dispatcher sentinel | 801B833C |
| M0351I | M0042I, M0092I | 80191124, 80191168 |

M0028I's M0031I exit is preceded by testing `g34 & 2`, then logical-not and
conditional skip (801A5F54 onward). M0031I sets that bit at8019E768/8019E780.
These conditions need live branch verification; adjacency does not establish
availability in every story state.

M0033I writes story g74=0x6C before its M0034I transfer. One M0359I transfer
writes g74=0x70, also written by M0034I's exit. In M0359I, the inspected
M0036I route writes g74=0x78. Its M0042I alternative writes0x88 and is gated
by g0 bit4 plus a dialogue selection; the bit's complete meaning is not yet
established here.

M0036I is shared across story states. Its g74=0x78 branch eventually writes
0x80 and g1=0x24, then requests M0000I. This is a sentinel, not a normal room:
`func_8001220C` handles token A8000048 by calling6ECEC and the scratchpad
entry8019234C. DAY1-8 translates the loader;8019234C remains a
Bootstrap_ReturnVoid definition in `pc_port/include/psx_compat.h`.
The original6ECEC starts by loading XA bank204
through blocking6CDA4, then loads overlay data. These functions and the
following transition must be restored and verified before any completion claim.
Do not index the room table with M0000I (that would use index-1).

The audit now also pins the full214-word6ECEC loader and its disc ranges.
Its table93168 contains sector offsets1276,1302,1547,1792,1975 relative to
PE.IMG LBA1013. It loads the first texture bank, one of two second banks
(selected by A77FC bit2000), then the transition overlay at8018EFF0.
The overlay is183 sectors atLBA2805, SHA256
`c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb`.
The253-word entry8019234C is a separate frame loop with30 static direct-call
sites, recorded in the audit output. Its full initialization/render/input
graph remains unported; recovering the loader alone will not close this gap.

Twelve executions of the original exit selector80192030 stop before its
first SDK call80074D28. They prove g74=78 selectsM0036I and g74=80 selects
M0351I, independent of the tested flag2000 and selection values0,9,FFFFFFFF.
This is a prefix test, not execution of the frame loop or exit cleanup.
M0351I's inspected script writesg74=88 before requestingM0042I; another
story branch writes140 before requestingM0092I. Branch traversal and the
actual Day1→Day2 presentation still need verification.

Current reproduction output: `local/live/day1-route-audit.json` (ignored).
Raw room data and disassembly remain ignored under `local/live/`.

## Rehearsal encounter and sewer entry

DAY1-11 extends the transfer inventory through the rehearsal scripts. M0023I
module0 reads actor event12 at801D7A64 before the story sequence that sets
g1=17 at801D7CB8 and g74=5E at801D7CC8, then transfers to M0367I. M0367I's
branch at801AA728 tests g74=5E; its dialogue/fade sequence writesg74=5F at
801AA840 and returns to M0319I at801AA850. M0319I's g74=5F entry sequence
then writesg74=60 at801A2334 and returns control.

Both M0023I and M0319I have an actor-event6 branch that displays dialogue86
and reads the resulting choice before their M0026I transfer. The inspected
exit instructions setg74=68 (801D7E5C /801A29F4), then request M0026I
(801D7E6C /801A2A04). The corresponding refusal branch returns control.
These are original script facts; event production, spatial interaction,
choice semantics, complete battle victory and live transfers remain to verify.

Source chunks: M0023I LBA13904/172sectors, M0319I LBA78563/61sectors,
M0367I LBA87312/78sectors. Checksums and35 immediate transfers are checked
by the audit. Ignored decoded inspection:
`local/live/rehearsal-route-modules.txt`, `M0023I-day1-script.json`,
`M0319I-day1-script.json`. Other M0367I branches belong to shared story
content and are not all counted as Day1 routes.

M0319I module7 supplies the sewer-entry interaction: original opcode77 at
801A359C tests the rectangle X693..1252, Z-999..-360, then gates on its
hit result and scratch20 before sending actor-event6 at801A3630. The script
sets scratch20 before posting the event.
A normal-input probe can approach X1000/Z-600 after the verified return;
no live sewer traversal is claimed yet.
