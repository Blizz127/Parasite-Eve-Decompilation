# `func_80079178` — screened SDK/COP2 wrapper

Disposition: `SKIP-SDK-LIBRARY-COP2`.

The span is `[0x80079178,0x800791D0)`, 0x58 bytes / 22 words in the generated
label geometry; the campaign pool's 21-word estimate excludes the trailing
alignment word. It ends with `jr ra`/`nop` at `0x800791C8/0x800791CC` and has
four exact-start callers, including `0x800C666C` and `0x800C6680`. The
preceding and following boundaries are real.

The body saves COP2 control registers 0, 2, and 4 with `cfc2`, loads three
words with `lw`, writes those controls with `ctc2`, loads vector operands via
`lwc2`, executes the handwritten COP2 operation at `0x800791AC`, stores three
results with `swc2`, restores the controls with `ctc2`, and returns.

This is the same handwritten SDK/GTE class already excluded by policy. It is
not a matching-C target, no source candidate was created, and the exact
matching-C count remains 335.
