# Escape-failure message through rendering and expiry

Stage119 connects original255E4 escape judgement to the controller join
restored in118 and the complete37870 message renderer. It uses original
executable text pointers selected by the producer, not replacement text.
Executable SHA1 is pinned to452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

Eight78-frame runs cover failed and blocked escape, both5BCB0 text variants,
and both message positions/initial packet banks. A prepared actor fixture
uses player level1 and enemy level7, HP45/45, and random seed1. The ordinary
attempt fails; the enemy's40000 flag produces the distinct blocked result.
The original255E4 sets D1CE and D1F8 and updates the attempt counter.
The fixture supplies valid menu and dialogue packet arenas, drawing tables,
and explicit actor/record state. The initial original371B0 message setup is
captured in the numeric seed; the test's behavioral comparison begins at255E4.

Each iteration executes complete original2AA24..2AA80 (excluding its parent
stack-restoring epilogue), then complete37870. The message is state2 through
frame74, with timer falling from74 to0. Frame75 clears the message and status
state and wraps the timer to255. Frames76/77 verify the subsequent idle state.
The624 frame checkpoints compare producer/controller state, message records,
menu drawing state, border and glyph packets, and ordering tables. A separate
checkpoint compares each producer result and its persistent state.

The native test uses the same original executable, numeric fixture and calls.
Packet generation does not by itself prove GPU pixels, actual window
presentation or a complete live escape attempt. The fixture does not invoke
the surrounding attack/escape animation or full battle dispatcher. Other
status-message producers and full Day1/Day2 acceptance remain open.

Reproduce with pe_escape_message_oracle.py; --check verifies the generated
retail_escape_message_cases.h. Native comparison is test_escape_message.h.
The initial probe with equal player/enemy levels succeeded; lowering the
prepared player's level selects the intended failure branch under seed1.
That is fixture selection, not a native runtime correction.

The integrated comparison exposed a native renderer omission on frame0:
FB subcodes>=10 were silently skipped. Original38338..38484 emits an extended
glyph at index(subcode+237), selects the second texture page when the computed
V exceeds240, and advances X by12 without proportional-width adjustment.
Native37870 now restores that branch. The original failure text exercises
FB19, FB0B and FB11; broader extended-glyph boundaries still need dedicated
coverage. FC/FD paths remain separately unported in the existing renderer.
The first failed sanitizer test leaked its fixture through the test assertion's
early return; that failure is not counted as a passing sanitizer result.

Final validation after the glyph fix:8 producer comparisons and624 rendered
frame checkpoints pass in normal and ASan/UBSan runs (one group passed,1310
skipped focused). Full CTest passes8/8 in97.88s. Native app rebuilt without
warnings; final fixture regeneration, Python and whitespace checks pass.
