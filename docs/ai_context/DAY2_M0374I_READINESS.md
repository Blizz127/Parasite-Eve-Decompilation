# M0374I resource readiness and music reset

Stage109 follows the model/animation work in DAY2_M0374I_ANIMATION.md.
Original authority is disc1 executable SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, with M0374I script SHA-256
`01b08019e76adebf20aeee364cf042b503fa2e4472885824bde7b8098a2cf33d`.

Module1 at801C55DC issues ED3100 (0xC1C) before its story-dependent music
selection. Original16910 routes this to16D94 and calls6914C(1). If that call
returns1, the opcode rewinds the next script PC by40bytes, stores task delay1
and returns0. Otherwise the opcode returns1 without changing PC/task delay.
The native ED handler previously returned1 immediately for this key, skipping
resource readiness. It now performs the original call and retry protocol.

Inspection also found a shared6914C mismatch: state bytes other than0/34/35/36
return0 via original69198 or691B4. Native previously returned1. This could
incorrectly keep callers waiting; the default return now matches the original.
No claim is made that an observed live freeze had this cause.

`pe_script_readiness_oracle.py` compares1016 full original ED3100 call graphs:
254 state bytes ×4 overlay/task variants. States34/35 (CD issue/poll) are
excluded from this oracle; state0 is supplied with D1A0 bit80 set, so the
preceding overlay initialization/callback is excluded. State36 uses empty
texture/archive directories. The cases prove the opcode wrapper, busy-start
state0→34 and40-byte retry, completed state0, empty state36 completion and
all other state-byte returns. The opcode's argument binding and next PC are
explicit fixture inputs, not a full M0374I VM/loader run. The existing native
BTL6_6914C_0x34_issue test separately exercises actual disc-backed host issue,
poll and completion, but is not original BIOS execution equivalence.

M0374I also issues EA217 at801C6F18 and801C7A08 before starting music0E.
Original15DAC162A0 calls6D24C, which resets six channel-state bytes B0DB2..B0DB7
toFF, clears overlay flagsF0, then calls86FF8 to enqueue audio commandF0.
Native15DAC previously ignored this key. New native6D24C reproduces the stores
and calls the existing86FF8 implementation; EA217 is now wired to it.
`pe_music_reset_oracle.py` compares48 complete original reset/FIFO graphs,
varying channel contents, flags and queue states. This verifies queued state,
not audible output or the full scene exit.

Numeric expectation headers are retail_script_readiness_cases.h and
retail_music_reset_cases.h. Native tests are test_script_readiness.h and
test_music_reset.h. No original script/asset bytes are added to tracked files.
Results and logs are recorded in ACTIVE_HANDOFF.md. Full scene construction,
pre-sender scene, dialogue closure, E8 branches, M0059I transfer and Day2 ending
classification remain unverified. The overall Day1/Day2 goal is unfinished.

Validation: normal and ASan/UBSan each pass1016 readiness cases and48 reset
cases in separate focused groups (1300 groups skipped per run). Full CTest
passes8/8 in84.63s. Both original oracle --check runs, Python and whitespace
checks pass; builds warning-free and native app rebuilt.
