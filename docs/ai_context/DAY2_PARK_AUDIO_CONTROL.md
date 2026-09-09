# Park return audio controls

Stage113 follows the transition work in DAY2_SHARED_PARK_TRANSITION.md.
M0040I uses EA305 at801C952C with duration60 and volume0, then a60-tick fade,
then EA314 at801C9560 before its D8 story assignment. Both commands were
silently ignored by the native15DAC default handler.

Original executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
EA305 selects original16384: it doubles arg1 using a32-bit shift and passes
arg2 to868AC. That leaf queues commandA9, masks duration to8 bits and volume
to7 bits. The native868AC implementation already existed; the script dispatch
now calls it with the original argument transformation. Thus the real room's
request60/0 produces duration120 and volume0 in the queued command.
EA314 selects164CC and calls existing87024, which queues commandF1. The
script dispatch now performs that call. Both original opcodes return1.

`pe_script_audio_control_oracle.py` executes complete original15DAC calls
through those leaves and the audio FIFO. The192 cases vary both keys, eight
durations including wrapping/high-bit values, four volumes and three queue
states. Numeric seed/expectation data are in
retail_script_audio_control_cases.h; native comparison is in
test_script_audio_control.h. The test compares selected audio/queue memory
and checks original command words and argument masks. It does not assert
that queued audio is audible, nor run the full M0040I scene/fade sequence.

The changes restore shared runtime commands used by this park return route.
FullDay1/Day2 scene, audio and release acceptance remain unfinished. Validation
results are recorded in ACTIVE_HANDOFF.md.

Validation:192 original/native cases pass in normal and ASan/UBSan runs
(1group passed,1304skipped focused). Full CTest passes8/8 in83.85s. Original
fixture regeneration, Python and whitespace checks pass. Builds warning-free;
native app rebuilt.
