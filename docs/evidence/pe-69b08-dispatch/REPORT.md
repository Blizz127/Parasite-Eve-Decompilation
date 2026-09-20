# func_80069B08 native port — the Disc-2 boot "boundary" is the retail disc-change screen

Branch `agent/dispatch69b08`, base `58ffdd4b`.
Worktree `/tmp/pe-agent-dispatch69b08`. Retail authority
`SLUS_006.62` SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Outcome in one line

`func_80069B08` is transcribed instruction-for-instruction and its
`Bootstrap_ReturnVoid` inline stub is gone (0 bootstrap stubs on the Disc-2
run).  But the function is the **retail disc-change / "insert the correct
disc" screen**: it blocks until the player opens the CD lid
(`CdlStatShellOpen`, status bit `0x10`) and inserts the disc the current
dispatch mode wants.  The Disc-2 verification image is exactly the
mismatch case (mode 1 wanted, Disc 2 inserted), so the faithful behaviour
is an indefinite wait.  The Disc-2 frame/polygon target ("same as the
stub baseline") is therefore **not achievable without faking** the disc
match or a shell-open event.

## What the function actually does

Body: `asm/disc1/56438.s` `0x80069B08..0x8006A0E8`, 0x5E0 bytes / 376 words.
Sole caller `func_8001220C` @ `0x800122A0`, guarded by sticky bit
`0x00100000` in `D_800B0CD8`; `a0` is the caller's dispatch mode (1 or 2);
retail `v0 = 0` is not consumed.

1. Two blocking PE.IMG sector reads from the base LBA `[D_800B0DD8]`
   (halfword ranges from the `D_800930D8` table): chunk 0
   `D_800930D8[0..2)` → `[D_800B0E64]` (71 sectors), chunk 1
   `D_800930DA[0..2)` → `[D_800B0E6C]` (1 sector).  Same
   restart-on-timeout issue/poll shape as 6B4F8/6AD40.
2. UI bring-up: `func_8003E974` (bit-table init), `func_800371B0([D_800B0E6C])`
   (message-data rebind), `func_800718D0` on the chunk-0 TIM,
   `func_80074F44` ClearImage (0,0,320,448), PutDispEnv for bank
   `D_8009CDDC`, `SetDispMask(1)`.
3. A 10-state CD-status state machine (jump table `jtbl_80011388` =
   `0x80069D10, D4C, D88, DA4, E30, E20, F04, F04, F04, F14`;
   `s2 >= 10` falls through to the draw tail):
   - state 0: `CdReady()==1 && queue()==0` → `func_8007EE84(8,0,0,-1)`,
     i.e. **CdlStop** (`8 = CdlStop`, PsyQ `libcd.h`); `sequence = s7`.
   - state 1: poll `func_8007F418(sequence, sp+0x18)`; status 2 →
     open result message, state 2; 5/6 → restart at state 0.
   - state 2: hold until `func_8007F788() & 0x10` — the CD response status
     byte's **`CdlStatShellOpen` (0x10, "once shell open")** — then state 3.
   - state 3: `CdReady()` 1 → `func_800374E8` + message id 3/4, 180-frame
     timer, state 5; `CdReady()==3` → timer, state 6.
   - state 4: `func_800698D4()` (disc mount / `PEDISC0x.IDF` + PE.IMG
     search) returns 0; `done` iff `mode==1 && D_800B0DCD&1` or
     `mode==2 && D_800B0DCD&2`; otherwise re-arm and show message ids 1/2/5.
   - states 5..9: 0xB4-frame countdowns into the message records.
4. Per-frame tail: `func_80037870`, DrawSync, VSync, `ResetGraph(1)`,
   PutDispEnv/PutDrawEnv for the current bank, `func_8007512C` on the
   (0x140,0x100,0x140,0xE0) rect, `func_800753B4` DrawOTag, toggle
   `D_8009CDDC`.
5. Exit: DrawSync, `SetDispMask(0)`, ClearImage, publish
   `D_800B0DCD = mode`, spin on `CdReady()==1`, publish
   `D_800B0DD4 = func_8007F7A8()`.

`D_800B0DCD` is written by `func_800698D4` from the inserted disc's
`\FMV1\PEDISC01.IDF` (bit 0) / `\FMV2\PEDISC02.IDF` (bit 1), and by
`func_8001220C`'s volume/`D_800A7918` gate to pick the wanted mode.  The
gate **skips** `func_80069B08` when the inserted disc already matches, so
the function only runs on a disc mismatch.

## Files changed

- `pc_port/game/boot/func_80069B08_port.c` (new) — full transcription.
  Guest scratch: retail's `sp+0x18` poll response and `sp+0x20` `-1`
  message-list terminator live at `0x801FF4C0` / `0x801FF4C8` (free
  `0x801FF040..0x801FFE00` band; `func_8007F418` stores there and
  `func_800375E0` reads the list, so both are real guest addresses).
  One host-only bound (documented in the header, same precedent as
  `func_8001220C`'s `PE_PORT_DISC_WAIT_LIMIT`): `PE_Port_ShouldStop()`
  returns from the state-machine loop instead of spinning forever when the
  host frame budget is reached.  It does not manufacture a return value or
  a screen effect.
- `pc_port/include/psx_compat.h` — the `func_80069B08` inline
  `Bootstrap_ReturnVoid` stub replaced by an `extern` declaration.
- `pc_port/CMakeLists.txt` — `game/boot/func_80069B08_port.c` added to
  `GAME_SRCS`.
- `pc_port/platform/pe_cdreg.c` — **wiring**: command `8` (CdlStop) added to
  the host drive's accepted commands and to the Pause-style completion arm.
  The function's first command is CdlStop; without this the run stops on
  `CD_device_unported_command`.  The host has no motor/seek state, so Stop
  shares Pause's modelled effect (end the read, ack + complete).

No callee of `func_80069B08` is unported, so the new file contains **no**
bootstrap boundary.

## Verification (all inside `/tmp/pe-agent-dispatch69b08`)

`cmake --build pc_port/build -j$(nproc)` — clean.

`ctest --test-dir pc_port/build --output-on-failure` — **11/11 passed**
(59.3 s).

`./pc_port/build/pe-native-tests` — **1394 run / 1394 passed / 0 failed /
0 skipped**.

`python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans` —
**check: OK**, 0 mismatched, 0 stale boundaries.

### Disc 1 route (must not regress)

```
PE_CARD=build/pe_card1.mcr ./pc_port/build/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" --headless --route-pad \
  --max-frames 80000 --trace /tmp/d1_after.log
```
`exit=0`, `[HOST] stop_reason=frame-limit`,
`bootstrap-return: 0`, `grep -c 'STUB:BOOTSTRAP_RET' /tmp/d1_after.log` = **0**
(matches the stated baseline of 0).  `func_80069B08` is never entered on
the matching Disc-1 boot.

### Disc 2 boot (the requested command)

Before (stub):

```
[STUB:BOOTSTRAP_RET] func_80069B08 (first invocation)
[FB]  vsyncs=1815 drawsyncs=609 presents=600 mask=1 main_iters=1
[HOST] stop_reason=frame-limit
[GPU] fills=597 ... polygons=529332 polygon_pixels=8532436
```

After (native):

```
(no func_80069B08 stub; bootstrap-return: 0)
[FB]  vsyncs=1824 drawsyncs=602 presents=600 mask=1 main_iters=1
[HOST] stop_reason=frame-limit
[GPU] fills=598 mono_rects=4 tex_rects=8865 moves=595 pixels...
       polygons=0 polygon_pixels=0
```

The stub requirement is met (`grep -c func_80069B08` = 0, bootstrap-return
0).  The frame count still reaches 600, but the polygons are **0**, not
529332: the frame budget is now spent on the retail disc-change screen and
the field never renders.  This is the retail outcome for this disc/mode
combination, not a stub or a fabricated value.

### State-machine trace (temporary `[DBG]` build, since removed)

```
mode=1 lba=000003f5 d0=8018efe8 d1=801ed800 t=0000 0047 0048 0057
state=0 ready=1 queue=0 status=02 B0DCD=02
state=1 ready=2 queue=1 status=02 B0DCD=02
state=1 ready=2 queue=1 status=02 B0DCD=02
state=2 ready=1 queue=0 status=02 B0DCD=02   (repeats forever)
```

`D_800B0DCD=0x02` (Disc 2 detected by `func_800698D4`), `mode=1` (wanted
disc 1; `D_800A7918=0 < 0x258`).  State 2 needs the status byte's `0x10`
(`CdlStatShellOpen`); the host drive reports `0x02` (standby) and has no
lid/cover input, so the state machine never advances to the disc test.

## Why the target cannot be met faithfully

- `func_8001220C` sets `dispatch = 1` because `D_800A7918 == 0` (retail
  initial value, read from the EXE at file 0x98118) and
  `func_800698D4` already proved `D_800B0DCD` lacks bit 0 (Disc 2 inserted).
- State 2 gates on `CdlStatShellOpen` (`0x10`); the host CD model has no
  lid event, and the host drive status is `2 | reading<<5`, which never has
  `0x10`.
- Even past state 2, state 4 can only finish when the inserted disc matches
  the mode; Disc 2 can never match mode 1.

Only faking the disc match, faking a shell-open event, or suppressing the
call could restore the stub baseline's 529332 polygons — all forbidden by
the task's "never invent behaviour / do not fake" rule.

## Residual boundary

None inside `func_80069B08` or its callees.  The only remaining host gap is
the CD drive's physical lid/cover event (no `CdlStatShellOpen` producer),
which is device-environment work, not a `func_80069B08` callee.

## Recommendation

Keep the transcription and the CdlStop wiring, and change the Disc-2
expectation: with the stub the port was silently skipping the retail
disc-change screen.  If a boot-to-field Disc-2 run is still wanted, the
port needs a host "matching disc / lid event" model at the drive layer (or
a Disc-1 image) — not a change in `func_80069B08`.
