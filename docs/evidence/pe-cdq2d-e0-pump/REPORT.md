# PE-CDQ2d — E0 delivery pump; BD4C reland; frontier at C89C

## The pump

`func_801924F8`'s E0 loop fires the installed DMA-completion
callback `func_8007C214` once per poll, before `func_80191B64`.
Retail populates streaming slots via DMA-completion interrupts
during this spin (7C214, installed by 81314's streaming arm);
the port has no async interrupts, so it delivers synchronously
— the 7ED58 synchronous-reset precedent. Recorded as the third
bounded adaptation in `pc_port/docs/RETAIL_ACCURACY.md`. E0
takes got_frame on the first poll, so cadence beyond that is
unobservable; the give-up path behind it is byte-identical
retail logic (lane -1 stays an unreached error arm).

## BD4C reland (from the MV1b report, verbatim)

`pc_port/game/boot/func_8010BD4C_port.c`, wired into CMake +
`pe_sdk.h`, stop stub removed. The E0 give-up hang that forced
the MV1b revert is gone by construction (E0 succeeds first
poll).

## What the live path proved (B54KY, real disc)

Pump publishes record-0 state 2 (`C0DC8=80142100`, indices 0,
dead `B0CC8` chain — measured at the boundary in CDQ2c);
7C484 promotes to 4; 91B64's BD0 steers past its 870F0 re-entry
(record word 0 < 0xFFFF limit latches `[DBD]`; taken C8C
publishes `[D111B0] = 0`, skipping the ClearImage fill);
got_frame → C89C stop. B54KY pins `func_8010C89C == 1` and the
promoted record (`[D0DFC] word == 4`).

## Test migrations (intent intact)

- B54KAD/AE: `CDQ2d_PlantStream` plants only prefix-untouched
  cells (`[D111B0] = 1` limit, `0x801B0000` decode buffer, FF FF
  terminator) — the prefix owns `[D111AC]`/`[C0DC8]` (7A214
  zeroes all record states) and the pump publishes state 2
  itself. Boundary `BD4C → C89C`, plus the record-0 promotion
  pin (`[801E0000] == 4`).
- B54KY: the 91FB8-flags group keeps its values (`[B0DBC]`
  stays 0 — the early conflation with `[DBD]` was caught and
  fixed); new E0-latch group (`[DBD] == 1`, `[D111B0] == 0`,
  both retail-true on these inputs, probed then probe
  reverted).

## Corrections during the rung

- B54KY's first failure blamed the wrong address (`[B0DBC]`
  vs `[DBD]`); fixed before commit — the suite never saw it.

## Verify

```
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
```

Both with `PE_DISC1_BIN`. Gateless: `1046 passed, 1 failed
(B54KY env), 17 skipped`. Normal CTest with disc: `100% tests
passed, 0 tests failed out of 2`. Fresh ASan/UBSan CTest with
disc: `100% tests passed, 0 tests failed out of 2`, zero
sanitizer diagnostics. MV1a oracle still green. Strict disc
run exits at `func_8010C89C` from `func_801924F8` (pump works
in production). Leaves 560; no src/YAML changes.
