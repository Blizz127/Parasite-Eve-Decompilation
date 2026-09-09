# Message choice-selection cursor

Stage123 restores the cursor packet omitted from native37870's FB9 control.
Original382C8..38338 writes the current text X and textY+12*signedSelection
into the preinitialized cursor sprite for the selected packet bank, then
links its packet at9EC70+28*bank into B0E38[bank]+4. It allocates no new
packet and does not advance the text position. Native now performs these
stores and linkage after its existing navigation and confirmation updates.

`pe_message_choice_oracle.py` pins executable SHA1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and executes complete37870 calls.
The2240 cases cover operands0..7/8/255, current selections0/1/6/7/127/128/255,
every combination of up/down/confirm, both banks and normal/wrapped text
coordinates. Two consecutive input frames give4480 checkpoints, comparing
selection/result bytes, record flags/count, cursor sprite position and links,
and surrounding glyph packets. Original371B0 initialization is captured in
the fixture. Message flags keep FF from closing the message during the sweep.

This verifies cursor packet generation and input state for explicit message
fixtures. It does not prove GPU pixels or a full script choice interaction.
Native opcode43 (17DE4) already reads the signed confirmed byte through37864;
stage124 verifies a real-script producer/renderer/consumer component in
[DAY2_M0040I_CHOICE.md](DAY2_M0040I_CHOICE.md).
Full Day1/Day2 scene and presentation acceptance remains open.

The first full regression run exposed an incomplete legacy BTL57 fixture:
it exercised FB9 with no initialized cursor or ordering table, which had
worked while cursor drawing was skipped. The test now calls371B0 and supplies
valid packet/ordering-table addresses. The native drawing behavior is retained.

The next run exposed the same missing rendering destinations in legacy
BTL59's field-frame choice test. It now supplies both banks;371B0 continues
to run through the field initializer under test. Both fixture corrections
preserve the original confirmation assertions.

Final validation:4480 original cursor frame comparisons pass in normal and
ASan/UBSan runs. Both corrected legacy tests also pass in both builds.
Full CTest passes8/8 in92.97s. Native app rebuilt without warnings; fixture
regeneration, Python and whitespace checks pass.
