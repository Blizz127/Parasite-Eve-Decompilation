# Message wait and confirmation controls

Stage122 corrects the native37870 FB6/FB7 cursor and counter rules using
original37F84..380CC. The previous implementation advanced a fixed two or
three bytes; the original sometimes leaves the cursor on a control operand
or consumes an additional byte, depending on the counter and pause state.

FB6 compares the per-render byte control counter with the record's four-bit
completed-control count. Confirmation advances matching completion state and
stops the current render; the original's subsequent subcode-byte check is
preserved. Without confirmation, matching controls stop without consuming the
subcode. FB7 stores its pause limit, increments the pause byte while waiting,
and on completion resets the pause and advances the record's completion
count with the original cursor movement. The shared byte control counter
advances once per control and remains shared across active message records.

`pe_message_wait_oracle.py` pins executable SHA1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and executes complete37870 calls.
Its360 eight-frame runs give2880 checkpoints across five adjacent/mixed
control streams, completion counters0/1/2/15, pause bytes0/1/255, three
confirmation patterns, and one/two active message records. Packet banks
alternate each frame. Comparisons cover message state, pause/counter fields,
rendered glyph packets and ordering-table links. Fixtures intentionally keep
messages open at FF so repeated controls remain observable.

Synthetic streams and explicit packet/record fixtures establish these control
paths, not every possible text stream, full control-counter byte wrap, GPU
pixels or a live dialogue scene. Choice control FB9 remains separately
incomplete and is the next renderer boundary. Full Day1/Day2 acceptance
remains open.

Validation:2880 original frame comparisons pass in normal and ASan/UBSan
tests (one group passed,1313 skipped focused). Full CTest passes8/8 in98.92s.
Native app rebuilt without warnings; fixture regeneration, Python and
whitespace checks pass.
