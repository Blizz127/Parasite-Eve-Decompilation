# Day 2 park progression and return-scene evidence

Stage105 follows M0191I and pins its neighboring M0195I/M0192I scripts plus
the world-map return scene M0037I. The reusable original-handler fixture now
accepts explicit extra persistent values, allowing persist34 to be traced
without substituting a Python implementation for the original predicates.
Reproduce with `pc_port/tools/pe_day2_park_progression.py`; detailed original
execution results stay ignored in `local/live/day2-park-progression-105.json`.

These are closed code regions with explicit scene boundaries. Actor creation,
message delivery, menu answers, animations, transfers and scene playback are
not skipped and are not claimed executed. In particular, returning through
M0037I does not yet prove the Day2 ending.

## M0191I conditions

Story is persist74; the secondary progression value is persist34. Indices
are decimal; story values and addresses are hexadecimal. Counter comparisons
below are signed, including negative test values.

Original module1 actor-selection block801B5204 chooses:

| Condition, in priority order | First scene boundary |
| --- | --- |
| story==C0 and persist34<=10 | 801B525C, first actor creation |
| C0<=signed story<=D0 | 801B5330, alternate actor creation |
| story==160 | 801B5380, shared later actor creation |
| otherwise | 801B53E0, resource release |

The separate progression gate801B55B4 uses the same first condition. When
true, it writes storyC8 and persist34=10, reaching801B562C before the next
menu/dialogue condition. Otherwise it reaches801B5730 with both values
unchanged. The preceding scene and following conversation are outside this
proof; reaching either gate is a separate scheduling requirement.

Original return block801B5A58 writes178 if incoming story==168, otherwiseD0,
then reaches the fade at801B5AAC. This is a shared conditional, not an
unconditional Day2 write. Its predecessor paths still require full tracing.

## M0195I secondary progression

The initialization block801AD574 checks signed persist34<13. If true, it
writes storyC0 and persist34=13; otherwise both are unchanged. Both paths
reach actor creation at801AD5BC. There is no story guard inside this block;
shared later-story inputs are therefore tested as well.

Consequently the C0/persist34=13 state written here does not satisfy M0191I's
C0/persist34<=10 gate. This explains why using story alone loses a distinction
in the park's initial progression. Other static writes set persist34 to14
and15, and the script contains a persistent-value wait; those producer/wait
chains remain to be executed.

## M0037I return scene

Original module2 gate801C94DC selects801C9504 only for storyD0. Other tested
stories reach801CA500, the shared later-story selector. The D0 scene's
asynchronous middle is not covered here.

The closed final block801C9AFC..801CA4F4 writes storyD8 and persist1=999,
prepares persist3 as `prior | 1E000 | (persist0 & 1 ? 800000 : 0)`, and reaches
an immediate M0000I transfer. This endpoint preserves the2000 flag, unlike
M0038I's C0 endpoint. Subsequent station entry is still a separate transition.

Read-ahead locates M0046I's D8->DA write at801AB6B4 and M0047I's DA branch;
these are next evidence targets, not yet executed by stage105. No Day2
terminal classification follows just from the D8 write.

## Newly pinned shared scripts

| Script | SHA256 | Immediate destinations | Story writes |
| --- | --- | --- | --- |
| M0037I | 1e580aa51863cff94d4b1885e1dc6126629ffb9ff1c957f3e67e4ee9d9d4862f | M0000I, M0271I, M0367I, M0000I | D8,282,29E,180 |
| M0192I | b4bbc95c4899374605162f79bca3c647c7d4ab58dee8ccdf5ca83dd41a6e4d4e | M0340I, M0196I, M0000I, M0037I, M0340I, M0196I, M0037I, M0000I, M0037I, M0000I | none |
| M0195I | fc8a4f77e40c5ef0096b93acd466ccc6475a454954091465d2e2f84f67ae7a54 | M0191I, M0191I, M0230I | C0 |

These static destinations are candidates for further classification; they do
not prove all destination rooms belong to Day2. M0195I->M0230I expands the
remaining park investigation beyond the initial entrance scripts.

## Verification scope

The tool tests story0..300 plus signed extremes, ten persist34 values around
10/13 and signed extremes, and both zero/all-one map masks. Results and job
completion are recorded in ACTIVE_HANDOFF. All17042 cases passed:7720 actor
gates,7720 progression gates,772 return selectors,772 scene gates,50 counter
floors and8 final blocks. Previous station/world-map audits also passed, with
normalized outputs identical to stages103/104 (ignoring the new empty fixture
metadata). Python/whitespace pass; all105 jobs finished. Runtime C is
unchanged, so stage102 remains the latest native build/regression result.
Full Day1/Day2 decompilation, optional-content classification and live
acceptance remain unfinished.
