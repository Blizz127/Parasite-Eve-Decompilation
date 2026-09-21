# M34 projectile movement, drawing and collision

The remaining M34 effect 8 children F830, FC54 and FDE4 are translated and
registered in the native child dispatcher. The M34 effect implementation now
covers all 970 source words from F00C through FF34. Connected F434 dispatch
still requires faithful retained stack inputs; this change does not establish
completion of the boss fight, Day 2, or whole-route retail fidelity.

## Source and behavior

Authority is original Disc 1 M34 C2, LBA16597, 100 sectors, base8018EFE8,
SHA-256 `0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a`.
Shared helpers are read from the original SLUS_006.62 executable, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

| Original span | Words | SHA-256 |
|---|---:|---|
| 8018F830..8018FC54 | 265 | 24834e38cd45b7112cc6454bc6ba3d25b8fb97b7d69e6980b1f832e9ee2e4e9e |
| 8018FC54..8018FDD4 | 96 | 7a72cc79d92ab7ccde3f453c0b27be11ec316c2f30d163af1209c8498e90e0a8 |
| 8018FDE4..8018FEE0 | 63 | 33cffea6f6830bf5b995c6178884846d82f66b87e6ebf46a8e1b74202689cd7b |
| 800C61A8..800C62DC | 77 | cd93641a32abd2094319d6332e7fb72b6d6e29c59686f77aa7b1204b7dcebf1d |
| 800C653C..800C6584 | 18 | 89af1d3b036a1f12fc73624617a5247b9df1140839e94500743f4a162a885185 |

`m34_boss_effect_port.c` preserves these behaviors:

- FC54 advances all three signed halfword coordinates with wrapping,
  increments size and the signed-byte timer, and raises fade only when its
  signed value is below 129. Inside the room polygon it attempts a trail
  allocation on every third signed timer value. Both original polygon calls
  remain, including the call after allocation. Outside the polygon it marks
  the record retired and clears the projectile's active byte.
- FDE4 selects the original texture settings, scales an identity matrix using
  the original constant, applies brightness and submits the complete shared
  billboard quad renderer.
- F830 configures rendering even for inactive records. Active records submit
  five quads in original order, preserve the unusual first-image subtraction
  of X velocity from all three coordinates, and preserve later Y overrides.
  Collision sets Aya's body+4C bit4000, the owner's body bit80000000 and the
  projectile record state2 only when the original owner classification is3.

`PE_EffectPointQuadC61A8` in `func_800CEB8C_port.c` rotates all four current
quad vertices with the original GTE operation, wraps translated X/Z, clears
Y, and calls the existing original triangle implementation twice. It retains
both calls and their scratchpad writes even when the first triangle hits.
The host matrix scaling also preserves the upper halfword written by original
78CC4's final word store.

## Independent comparison

```
python3 pc_port/tools/pe_m34_children_oracle.py \
  --build-dir /tmp/pe-day2-release --write-header
```

**303 original/native cases PASS, 1,540 unique instruction PCs.** The original
interpreter executes complete callees with no replacements. Every executed
instruction and following word is checked against the original EXE/overlay.
Each case compares every RAM byte below1FE000 and all1,024 scratchpad bytes.
Original CPU stack space is excluded. Every changed byte in compared original
RAM is additionally checked against the regression hash ranges.

Coverage includes signed timer and fade boundaries, wrapping coordinates,
polygon edges and empty polygons, all six owner classes, a full child pool,
allocation offsets including wrapping and negative offsets, both GPU banks,
brightness and size extremes, texture flips, depth rejection, and inactive
projectiles. The run observes31 original collision hits and packet counts
0,4,5 for main projectile drawing. Submitted packet bytes and ordering-table
links are compared directly; the claim does not extend to rasterized pixels,
all final GTE flags/registers, or hardware timing.

The initial movement/trail-only run passed196 cases and917 PCs. Its first
attempt failed to compile the standalone harness because `game_port.h` was
missing; that harness include was corrected. No production correction was
needed for the subsequent passing comparisons. Final oracle log:
`/tmp/pe-m34-allchildren-oracle.log`.

The generated `retail_m34_children_cases.h` uses repeated-word runs and sparse
case patches. `test_m34_children.h` reproduces all303 original expected hashes,
including scratchpad, inside the native suite. Release and Debug full builds
PASS. The native suite reports **1,397 run, 1,397 passed, zero failed/skipped**.
All **10/10 non-route CTest checks PASS in61.11seconds**, with the native
suite taking59.08seconds. Logs:
`/tmp/pe-m34-allchildren-tests-build.log`,
`/tmp/pe-m34-allchildren-debug-build.log`,
`/tmp/pe-m34-allchildren-ctest.log`, and stable
`/tmp/pe-m34-allchildren-lasttest.log`. All build, oracle and test processes
are terminal. No production changes followed these passing checks.

No connected replay was repeated for this change: F434's unresolved retained
inputs remain the earlier route boundary. No gameplay RAM injection,
checkpoint restoration, matching-source edits, or executable changes.
The next required work is the
[initializer's caller-stack history](M34_PROJECTILE_INITIALIZER.md).
