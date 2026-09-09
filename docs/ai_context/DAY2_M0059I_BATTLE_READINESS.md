# M0059I battle request and readiness

Stage117 connects two actual M0059I module7 request/wait sequences to the
complete original mode6 readiness controller. The script SHA256 is
`d61b54ff563db4c833af7f2e2dd198216170e3aba111d855f08aec8a0d68996a`;
the shared loader pins the original executable SHA1 and reads the script
from the supplied Disc1.

The first sequence starts at801A6934 and the second at801A6F78. Each polls
opcode94 until mode0, executes opcode89 to request mode6, then polls until
mode7. The controller8002BC90 settles pending amount timers and actor command
states, performs its cleanup and audio/UI work, and publishes mode7. The
following VM call sends the original messages and forks the task at801A709C.
The parent ends after6A24 or7068, retaining next-PC6A2C or7070 respectively.

`pe_m0059i_battle_ready_oracle.py` executes full original VM calls and, when
mode6 is requested, the full original2BC90 graph. Twelve runs cover both
script sequences, both packet banks and pending timer0/1/3. Each runs eight
iterations with comparisons after the VM and after the controller, for192
checkpoints. The original publishes mode7 on frame0/1/3 respectively.
The native test loads the original script chunk and compares the same actor,
task, battle, UI, effect, audio and global ranges. It requires no stub or stop.

Prepared battle actors/resources come from the existing mode6 oracle fixture;
the harness adds an explicit script actor, task pool and persistent flag0.
The harness selects the current task each iteration and calls2BC90 only in
mode6. It does not run the entire299CC dispatcher, live enemy simulation,
mode7 combat, victory, scene loading or rendered presentation. This is
readiness evidence, not proof of winning a battle. The original/native
comparison required no runtime correction.

Read-ahead found remaining native omissions in2A7F8_join_cut: original
2AA24 calls34DE0 when D1CE is nonzero and mode is0; it also calls67CBC when
signed-half D2A4 is nonzero. Native currently only calls33A40 for D244.
Original34DE0..34F10 manages a timed message through374E8,5BCB0,37454,
375E0,5E894 and61C34. These join paths require implementation and verification.

Normal and ASan/UBSan pass all192 checkpoints in one focused group, with1308
other groups skipped. Original fixture regeneration, Python and whitespace
checks pass. Builds are warning-free; runtime remains stage115 unchanged.

Full CTest passes8/8 in84.07s. The next omitted67CBC call changes field/menu
flags; the earlier progress description of it as input clearing was imprecise.
