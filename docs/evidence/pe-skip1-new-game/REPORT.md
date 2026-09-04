# PE-SKIP1 — skip-movie New Game entry (2026-09-03)

## What landed

`--skip-movie` is a documented HOST_ADAPTED dev entry so field work
can proceed while the opening FMV / title menu stay untranslated.

- `func_801909B4` returns selector 1 after the overlay prefix (logo,
  `func_80192CE8`, and the title tail do not run).
- `func_8006E9A0(1)` still publishes the retail New-Game token
  `0xA80830C8`. Under `--skip-movie` only, the fade poll honors the
  host stop and forces `CFEE=1` after the first tick so a re-armed
  fade cannot trap the publish.
- Windowed `port_main` installs `HostWindow_PadRaw` as
  `PE_Port_SetPadSource`; the field tick writes those active-low Sony
  bits to `D_800BE9A2` before `func_8003EB04`. Tests may install a
  scripted source.

## Verify

```
PE_TEST_FILTER=SKIP1 ./pc_port/build/pe-native-tests
# SKIP1_909B4_returns_new_game PASS
# SKIP1_1220C_publishes_new_game PASS (disc): D_8009D280 == 0xA80830C8
#   and overlay+0x18C dest head == 0x8A1C (M0431I chunk2)
PE_TEST_FILTER=PAD1_source ./pc_port/build/pe-native-tests  # PASS
PE_DISC1_BIN=<Disc1.bin> ./pc_port/build/pe-native-tests
#   Results: 1073 run, 1073 passed
```

Production: `--headless --skip-movie --disc-image <Disc1.bin>` reaches
the field tick (no `BOOTSTRAP_RET` on the first New-Game dispatch);
`--max-frames` then stops the host inner loop.

Default boot (no `--skip-movie`) is unchanged: frontier remains
`func_8010C89C` from `func_801924F8`.

## Next

Field tick / spawn / collision / BG+model presentation so Aya is
visible and walks in M0431I. FMV/title remain deferred.
