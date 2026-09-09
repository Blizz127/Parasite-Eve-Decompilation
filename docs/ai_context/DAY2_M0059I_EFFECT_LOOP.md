# M0059I repeating effect script

Stage116 verifies the actual M0059I module2 component starting at8019FDC4:
ED2401 sets RGB192/8/8, ED2402 creates a mode1 effect at
(-1488,-350,-3588) with size30 and speed10, opcode02 waits25 ticks, and
opcode00 returns to the color setup. This connects the command allocation
restored in114 to the full update/draw graph restored in115.

The pinned script SHA256 is
`d61b54ff563db4c833af7f2e2dd198216170e3aba111d855f08aec8a0d68996a`.
The executable SHA1 is checked by the shared station loader. The oracle loads
the real script chunk from the supplied Disc1 and runs complete original
17018 VM calls followed by complete E01BC effect updates and packet drawing.
The harness supplies the actor/task, camera/projection, effect pool and packet
arenas. It selects the actor's task at each frame, alternates packet banks,
resets the packet cursor and initializes the ordering tables. These are
explicit component inputs, not a claim of complete scene construction.

Four200-frame runs vary initial packet bank and two camera translations.
All produce spawns at frames0,25,50,75,100,125,150,175, reach three active
effects, and retire older effects while the loop continues. Every frame
compares VM/global state, actor/task records, pool state, packet memory,
ordering tables and scratch bytes0..37. The native test loads the same
original script from Disc1; no asset or executable bytes are stored in its
numeric expectation header.

The first oracle attempt selected the task only once, so later VM calls had
no task to run. Correcting that fixture to supply the per-frame task produced
the repeating original sequence. This was a harness issue, not a native
runtime correction. Full M0059I construction, GPU images, other concurrent
script tasks and battles remain outside this component's evidence.

Reproduce with pe_m0059i_effect_oracle.py (--check verifies the generated
header). Original evidence is local/live/m0059i-effect-116.json and
oracle-day2-116.log. Native comparison is test_m0059i_effect.h.

Validation:800 checkpoints pass in normal and ASan/UBSan tests (one group
passed,1307 skipped focused). Full CTest passes8/8 in85.25s. Fixture
regeneration, Python and whitespace checks pass; builds are warning-free.
No native runtime change was needed; latest app runtime remains stage115.
