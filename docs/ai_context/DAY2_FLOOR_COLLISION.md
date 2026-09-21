# Retail floor collision and the m0005i doorway

The native floor path had three differences from original `8001AE40` and
its callees in `asm/disc1/AF84.s`:

- It always cleared the cached-wall contact and searched edges again. Retail
  first compares two NCLIP results and four extent gates (`1AF50..1B174`).
  The first xmax gate uses z, while the others use z-radius. Extent widths
  wrap to signed 16 bits. The port now preserves those branches.
- It limited slide displacement to radius+2. On a diagonal wall, the required
  axis displacement can exceed that limit. `1C7DC` searches until clearance;
  the port now does likewise.
- It rejected a zero triangle-crossing result. Retail `1B3A0` proceeds with
  loads from physical RAM zero and publishes the zero triangle pointer.
  The port now translates those record reads to the base of guest RAM.

`src/` and the matching manifest were not changed. This is native behavior
work, not a new matching-C claim for these still-assembly routines.

## Original execution evidence

`python3 pc_port/tools/pe_floor_collision_oracle.py --check` executes the
complete original collision graph in the test-only MIPS interpreter. It
verifies the original Disc 1 executable SHA-1 before running. The 288 cases
use synthetic 2D/3D meshes and test contact, movement away, corners, sign
changes, internal seams, different radii, NPC rejection, and null crossing.
The native `COL2_retail_floor_collision` test compares RAM hashes against
`retail_floor_collision_cases.h`. No original instructions, maps, or captured
retail RAM are embedded in the header. GTE state is not part of this claim.
Invalid mesh guards and recursion/visited bounds remain native adaptations.

A local diagnostic captured the naturally reached m0004i RAM at frame 8899,
then compared original/native collision from the same state for repeated
(-231700,-231700) fixed-point steps. The old implementation first diverged
at step 65. With the cached-wall gate, all 600 steps matched guest RAM outside
original code and stack. This isolated probe is not a route replay. Its RAM
files remain under `/tmp` and are not committed.

## Connected route

The harness now defaults to 9600 frames and continues its previous four pad
stages with raw `FFDF` at frame 8990 and `FFEF` at 9140. This approaches the
left door across connected triangles, enters the op77 volume at `801B6B74`,
and lets the room script send mailbox payload 4 and transfer at `801B614C`.
No coordinates, mailboxes, or story flags are injected.

Observed trace with the collision fix:

- m0377i at frame 6976, m0378i at 7081, m0004i at 7929.
- **m0005i (`A80002C8`) at frame 9300**, story `18`, persist[1] `4`.
- At frame 9600, module 4 waits at `801B1CF8` for its central rectangle
  (`801B1C14`, x=-820..995, z=-202..2328).

The regression requires 15 observed milestones and the current m0005i token
and module-4 PC. The milestone table is an observation set, not an ordered
assertion; its output now says so. Movies and the opening menu still use the
four documented HOST_ADAPTED skips. This is still Day 1, not Day 2 completion.

`PE_ROUTE_PAD_SEQUENCE` accepts up to 32 increasing `decimal-frame:hex-pad`
pairs, separated by commas; an empty value disables the continuation.
The normal periodic Cross pulse applies to these masks too. The sequence
is printed before the run. `PE_ROUTE_RAM_DUMP` optionally writes the final
2 MiB for local differential research; the harness never reads a checkpoint.
`PE_ROUTE_AYA_EVERY` now retains its requested interval.

**Historical failure, now resolved by the [script polygon boundary port](DAY2_BATTLE_BOUNDARY.md):**
A further probe with `9600:FF7F` reached the central encounter path but aborted
on a guest write to address `00000068`. The backtrace resolves to
`80035558_walk_cut -> 800299CC_damage_entry_cut -> 80021DE0`: the latter
reads Aya pointer `8009D254` as zero and writes actor+104. The next work is
to establish why the encounter has an active command queue without Aya,
using original battle initialization and input gating. The failure is not
hidden by the route assertion.

## Validation

Original oracle: 288 graphs and generated-header reproducibility PASS.
Independent native/original fixture comparison: all 288 PASS.
The existing full candidate remains byte-identical to the original, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (cmp and SHA-1 rechecked).
The prior full retail rebuild remains applicable because no matching inputs
changed. Full native CTest: 10/10 PASS; native tests: 1376/1376, zero skips.
The default 9600-frame m0005i regression passes.

Logs: `/tmp/pe-floor-{full-build,ctest}.log`,
`/tmp/pe-floor-door-turn.log`, `/tmp/pe-floor-chain-fixed.log`, and
`/tmp/pe-floor-encounter-backtrace.log`.
