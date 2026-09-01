# PE-B54K-AK — movie CD idle wait

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the complete ready/queue drain loop between stream-state
initialization and the next CD command. Both callees were already complete;
no controller state is forced to make the loop exit.

## Retail identity

```text
loop                  [0x80192770,0x80192790) / 8 words
caller prefix         [0x801924F8,0x80192790) / 166 words
caller-prefix SHA-256 f82288e0a5e751bb7399cd1c34d9d60904de59150b9f98ad0f539d090dbac8eb
```

All loop words are:

```text
80192770 0C01FDCB  jal   func_8007F72C     # CdReady
80192774 00000000  nop
80192778 1450FFFD  bne   v0,s0,80192770    # s0 == 1
8019277C 00000000  nop
80192780 0C01FDDE  jal   func_8007F778     # queue-depth getter
80192784 00000000  nop
80192788 1440FFF9  bnez  v0,80192770
8019278C 24040002  li    a0,2              # next-call argument
```

The first following word at `0x80192790` is `lui a1,0x801D`, beginning the
argument setup for unresolved `func_80080D5C`. Therefore the carve contains
the whole wait and no part of the next command.

## Native contract

The source preserves the two-level retail test:

```c
do {
    while (func_8007F72C() != 1) {
    }
} while (func_8007F778() != 0);
```

`func_8007F72C` derives ready state from the represented retail CD lane and
`func_8007F778` reads the guest queue-depth word. The movie source contains
no store to either authority. This is a real wait, not a hardcoded success.

## Controls

- Real Disc 1 reaches the loop after synchronous retail-derived CD reset and
  leaves ready state 1 and queue depth zero unchanged.
- The independent FMV2 synthetic fixture reaches the same loop from its own
  disc image and state setup.
- Source inspection rejects writes to `D_8009B574` and `D_800A3608` in the
  translated movie function.
- The oracle pins both backward branches, call order, and the next boundary.

## Verification

```text
B54K-AK independent oracle: PASS
B54K-Y real-disc tests:     2/2 PASS
B54K-AE synthetic test:     1/1 PASS
native suite:               992/992
normal CTest:               2/2 PASS
fresh ASan/UBSan CTest:     2/2 PASS
strict real-disc exit:      1
strict frontier:            func_801924F8_80192790_cut
retail EXE SHA-1:           452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No CD command is issued, no stream data is read, no callback is invoked, and
no scene, scheduler, story, persistence, or destination state is planted.

```text
FUNC_801924F8_CD_IDLE_WAIT=COMPLETE
CD_READY_AUTHORITY=READ_ONLY_NOT_FORCED
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192790_cut
NEXT_ARTIFACT_FREE_RUNG=func_80080D5C_and_func_80081314_command_path
```
