# PE-B54K-AL — blocking CdlSetloc production arm

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung crosses the first blocking CD command in the authenticated movie
initializer.  It implements only the retail-proven command-2 path and stops
before the following stream-read wrapper.  It does not claim a complete
libcd command engine.

## Retail identity

```text
wrapper                 [0x80080D5C,0x80080DC4) / 26 words
wrapper SHA-256         78dab759fceb6569cc6d7b903db2e16e026f46b39c75a32f351289c1af6db824
caller prefix           [0x801924F8,0x801927A0) / 170 words
caller-prefix SHA-256   fb44b0aeb6ab2d3e6c52cf5242603e038dc32cd096062ade16a0b6cf713c9f39
retail EXE SHA-1        452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The wrapper masks the command to a byte, submits it through
`func_8007EE84`, polls the returned handle through `func_8007F418`, and
returns one exactly when the terminal status byte is `2`.  Its canonical
return is `jr ra` plus stack-restore delay slot; real instructions bound both
sides.  The overlay contains three direct calls, including this call at
`0x80192798`.

The executable's own command-name table contains `CdlSetloc` at raw offset
`0x22BC`, in command-number order, proving that command `2` is CdlSetloc.
The production call is exactly:

```text
80192790 3C05801D  lui    a1,0x801D
80192794 24A50DC4  addiu  a1,a1,0x0DC4
80192798 0C020357  jal    func_80080D5C
8019279C 27A60040  addiu  a2,sp,0x40
```

Thus the input is the four-byte `CdlLOC` returned by the already-complete
`DsSearchFile`.  A full suffix scan through the caller's return at
`0x8019292C` finds no load from stack offsets `+0x40..+0x47`: the eight-byte
controller response is dead.  The first following word at `0x801927A0` is
the `lui a0,0x801D` that begins the unresolved `func_80081314` call.

## Native scope

The synchronous host CD model retains the exact four CdlLOC bytes as
value-only controller state after proving:

- command byte is exactly `2`;
- the caller does not request response bytes;
- parameter range is valid guest RAM;
- ready lane is `1` and queue depth is zero;
- Disc 1 is present;
- minute/second/frame bytes are valid BCD and resolve inside the disc.

It leaves retail ready and queue authorities unchanged.  Any other command,
any response-consuming caller, absent disc, invalid BCD, or out-of-range LBA
returns zero without changing the represented Setloc state.  This bounded
arm is enough for the proven movie caller because its response is dead; it
is not presented as complete `func_80080D5C` coverage for arbitrary callers.

## Controls and verification

The focused synthetic-disc test resolves `\FMV2\FMV018.STR;1` to LBA 42,
passes its exact CdlLOC through command 2, and checks retained bytes, LBA,
ready state, and queue depth.  Negative controls reject command 3, a
non-null response consumer, and malformed BCD while preserving response and
Setloc canaries.  The independent oracle authenticates both binaries, all
26 wrapper words, the four caller words, dead-result liveness, and the next
boundary.

```text
B54K-AL independent oracle: PASS
B54K-AL focused test:       1/1 PASS
native suite:               993/993
normal CTest:               2/2 PASS
fresh ASan/UBSan CTest:     2/2 PASS
strict real-disc exit:      1
strict frontier:            func_801924F8_801927A0_cut
```

No stream read, callback delivery, MDEC completion, movie frame, scene,
scheduler, story, persistence, or destination state is fabricated.

```text
FUNC_80080D5C=PRODUCTION_COMMAND2_ARM_ONLY
COMMAND_2=CdlSetloc_RETAIL_NAME_TABLE_PROVEN
RESULT_BUFFER=DEAD_IN_COMPLETE_MOVIE_CFG
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_801927A0_cut
NEXT_ARTIFACT_FREE_RUNG=audit_func_80081314_and_func_8007F0C8_mode_1E0
```
