# SPU mode-register writer

Stage126 translates complete original8008D140..8008D610, one dependency of
the SPU mode switch currently blocking opening playback. The32-bit mask at
attributes+0 selects halfwords atattributes+4..42 for stores at
D_8009B3FC+1C0..1FE. A zero mask writes all32 registers. Each selected store
reloads the destination base before reading its source, preserving original
ordering. The source mask is loaded once. This helper is not yet wired into
the unfinished8CF70 mode-switch graph.

pe_spu_mode_register_oracle.py pins executable SHA1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b and executes full original8D140.
The204 cases cover zero/full/alternating masks, each single selected bit,
each single cleared bit and three halfword patterns. Both source data and
an explicit RAM register destination are compared, including untouched bytes.
This validates address/value selection; it does not emulate physical SPU
register effects or prove DMA clearing, events, mode switching or audio.

Native implementation:platform/pe_stream.c, declaration:platform/pe_sdk.h.
Numeric fixture:retail_spu_mode_register_cases.h; test:test_spu_mode_register.h.
The existing opening regression remains the priority. After implementing
8CB54/8CF70/8D610 and the completion path, rerun the opening test and full
regression suite before rebuilding the requested Banshee update.

Build125 Release binaries/packages were captured before this helper change;
this stage's source is not included in those archives. Publication remains
pending the known opening blocker or an explicit experimental-build choice.

Validation:204 original comparisons pass normal and ASan/UBSan. Both builds,
fixture regeneration, Python compilation and whitespace checks pass. The
known stage125 opening failure remains; no full regression rerun is claimed.
