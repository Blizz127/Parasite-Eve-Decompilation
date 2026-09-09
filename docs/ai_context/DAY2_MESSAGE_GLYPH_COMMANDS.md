# Message icons and extended glyph commands

Stage120 restores the native37870 renderer's missing FC/FD glyph commands
and FB icon subcodes0..3, and broadens verification of the FB extended glyph
path restored in119. These shared message commands affect both days.

Original37E6C..37F54 draws FB0..3 as12-pixel icons at U=64+12*subcode,
V=halfword9166A, using page91670 and CLUT91672. Original38484..385C4 maps
FC's operand to glyph index operand+52;385C4..38704 maps FD to operand+308.
Both use U=(index%21)*12, V=(index/21)*12 truncated to packet bytes, page91660
and CLUT91662. Unlike ordinary glyphs, each advances X by12 with no
proportional-width adjustment. The shared native packet helper now implements
these paths alongside FB>=10. FB extended glyphs retain their different
index/page-switch rules recovered in119.

`pe_message_glyph_oracle.py` pins executable SHA1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and executes complete original37870.
The3048 cases cover all256 operand values of FC and FD, FB0..3 andFB10..255,
two packet banks and two color/position configurations. Each stream places
two tested glyphs beside an ordinary character, then a newline and another
ordinary character. This checks both fixed/proportional spacing and newline
reset, texture-page transitions, UV byte truncation, coordinate wrapping,
colors and ordering-table links. The fixture captures original371B0 setup
and supplies synthetic message/packet memory. Expected headers contain
numeric state and fingerprints only.

This is full-renderer execution for the specified streams, not proof of all
text control commands, a GPU image, or complete Day1/Day2 presentation.
FB4..9 are excluded from the tested operand sweep because they are controls;
FB4/5 color changes are used in the fixtures. Other numeric/choice/control
branches still require coverage and implementation where missing.

Validation:3048 original comparisons pass in normal and ASan/UBSan tests
(one group passed,1311 skipped focused). Full CTest passes8/8 in94.70s.
Native app rebuilt without warnings; fixture regeneration, Python and
whitespace checks pass.
