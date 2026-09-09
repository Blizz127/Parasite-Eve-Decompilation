# M0040I dialogue choice integration

Stage124 connects actual M0040I message 1 to the restored message renderer,
confirmation result, VM opcode43 reader, and both outgoing script branches.
The original executable SHA1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; script SHA256 is
`6071b63b3d50af50a5b1cab46bd5bd659baa45ea2ac015d6a994f63ca832784f`.
The test reads the original disc chunk and its text directory at801D0800.

`pe_m0040i_choice_oracle.py` starts module2 at801C8FEC with explicit actor,
task, input and rendering destinations. Complete original17018 and37870
calls execute each frame. The eight runs cover selections0/1, initial packet
banks0/1 and confirmation delays0/4. Their44 checkpoints compare selected
RAM, including message records, cursor/glyph packets, task/local state,
audio command queue and fade globals. Original371B0 initialization is
captured in the seed; native initialization is not independently exercised.

After confirmation, opcode43 at801C9004 stores the selected result in local4.
Selection0 executes the30-tick wait at801C9078 and leaves taskPC801C9084.
Selection1 executes EA305 at801C952C, starts the60-tick fade at801C954C,
and leaves taskPC801C9558 at the fade poll. Both paths match the original
without additional native runtime changes. The cursor restoration from123
is therefore exercised with an actual script producer and consumer.

Normal and ASan/UBSan focused tests pass all44 checkpoint comparisons.
Fixture regeneration and Python compilation pass. Full CTest passes8/8 in95.77s; both test builds are warning-free. See
local/live/ctest-day2-124.log. The runtime app remains the stage123 build.

This is a component test. Scene construction, the preceding story gate,
continuation after the next wait, fade completion, later EA314 command,
GPU pixels and audible playback remain outside its scope. It does not
establish full M0040I scene or Day1/Day2 acceptance.
