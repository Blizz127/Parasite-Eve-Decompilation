# `func_800791D0` — screened SDK/COP2 wrapper

Disposition: `SKIP-SDK-LIBRARY-COP2`.

The span is `[0x800791D0,0x80079228)`, 0x58 bytes / 22 words. It ends in
`jr ra`/`nop` at `0x80079220/0x80079224`, has an exact-start direct caller at
`0x800C5754`, and has real function boundaries on both sides.

Its body is the structural twin of `func_80079178`: it saves COP2 control
registers 0, 2, and 4 with `cfc2`; loads and installs three control words;
loads vector data through `lwc2`; executes the alternate handwritten COP2
operation at `0x80079204`; stores three results through `swc2`; restores the
saved controls; and returns.

The required COP2 operations are outside sanctioned ordinary C. No source
candidate was created, YAML/build state is unchanged, and the matching-C
count remains 335.
