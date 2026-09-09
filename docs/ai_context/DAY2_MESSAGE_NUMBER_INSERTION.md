# Numeric insertion in messages

Stage121 restores FB8 in the shared native37870 message renderer. This command
was silently skipped even though375E0 already prepared numeric digit rows.

Original380CC..381F0 reads a signed count from digitrow+5, emits digits from
row+count-1 back to row, and advances the row pointer by6 after every FB8,
including an empty row. Each digit emits a28-byte packet with U=12*digit,V=0,
page91650/CLUT91652 and current message color, advancing X by12. Native now
performs these operations and resets the row pointer to record+1A for each
record render. Existing glyph packet construction is shared with the icon
and extended-glyph paths; ordinary proportional-width glyphs are unchanged.

`pe_message_number_oracle.py` pins executable SHA1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and executes complete375E0 followed
by complete37870. Its336 cases cover zero through five supplied halfwords,
values0,1,9,10,99,100,999,1000,9999,10000,32767,32768,65534,65535,
both packet banks and two color/coordinate configurations. Rotated lists
exercise multiple decimal lengths, negative inputs, early -1 termination,
empty rows and all five FB8 insertions. Negative values retain the original
digit conversion behavior; the native port does not invent a minus sign.
Each setup result and rendered result is compared independently.

The fixture captures original message initialization/geometry setup and
supplies synthetic text, lists and packet arenas. Expected headers contain
numeric seeds/fingerprints. This verifies setup-to-render behavior for the
stated lists, not a live scene, GPU pixels, arbitrary corrupted digit counts,
or remaining choice/wait controls. Full Day1/Day2 acceptance remains open.

Validation:336 original setup/render comparisons pass in normal and
ASan/UBSan tests (one group passed,1312 skipped focused). Full CTest
passes8/8 in95.66s. Native app rebuilt without warnings; fixture
regeneration, Python and whitespace checks pass.
