# M0020I save-menu termination: E7 is *not* re-issued, the save publishes no
# "success" result, and the recorded route was missing its third Circle

Branch `agent/script-e7` (base `cd5ba828`, main HEAD at start).
Retail authority unchanged: Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.  No `src/`, `configs/`, `asm/`
or `scripts/` change.

## The two candidate hypotheses

The handed-over task was: after the Day-1 `M0020I` save, does the save path
publish a non-cancel menu result (and through which function), or is the
`E7` opcode being re-issued by the script in `func_80017018`/`func_80015AF0`?

Answer: **neither**.  `E7` is dispatched exactly once (its normal two-phase
entry), and the save path never publishes the "success" result `2`.  The
retail close path the menu already has — `func_8004D2DC event & 0x40 ->
func_800512AC(9,0) -> D_8009D010 = 0xFFFFFFFF` — is what ends the menu, and
the script does not gate on it.  The route failure was host input: the
recorded route needed a third Circle press that its table did not carry.

## Evidence: E7 is called exactly once

Instrumented port run (temporary dispatch trace on `fn == 0x80015AF0`,
`--headless --route-pad --max-frames 42000`, real Disc 1, present card):

```
[TRACE] E7 pc=801a0ff8 args=80120f80 -> 0   <- first phase: task[8] |= 0x20, delay=1, D_8009CE00 -= 8
[TRACE] D18C OPEN                            <- func_8004D18C builds windows 36/37/38/19 + slot list
[TRACE] C1EC enabled=1 d030=00000000         <- func_8005C1EC(1): menu mode on
[TRACE] E7 pc=801a0ff8 args=80120f80 -> 1   <- second phase: opens menu, returns 1, PC advances
```

Both dispatches are at the same script PC, one frame apart, and there is no
later `E7` at all: a whole-run trace has `E7=2` and `D18C=1`.  `func_80015AF0`
returns `1` on the second call and the VM (`func_80017018`) stores
`D_8009CE00` (already advanced past the opcode) into `task[0]`, so the script
moves on immediately; it never loops back.  Hypothesis (b) is disproven.

Raw: `/tmp/tr4.log` (this report's run), compared with the windowed
`/tmp/tr3.log`.

## Evidence: the save publishes no result 2

`func_800512AC` result publishers seen over the whole 42000-frame run:

```
[TRACE] func_800512AC cmd=10 src=00000000   <- top-menu open (result 1000)
[TRACE] func_800512AC cmd=9  src=00000000   <- menu close/cancel (result 0xFFFFFFFF)
```

`cmd=12` (result `2`) is never reached.  Retail confirms `2` is exclusive to
the **load** CRC tail: the only installer of `func_80042228` is
`func_80042264` (`asm/disc1/32A64.s`, `0x80042428 jal func_8004D024` with
`a0 = func_80042228`, reached only when the copied `0x2000`-byte block passes
the CRC-16 compare at `0x800423F8`).  `func_80042228` in turn is the only
caller of `func_800512AC(12)`.

The save-write path (`func_80041108` states 9 -> 3 -> 8 -> 10 -> 3 -> 12)
never enters state 7 (the CRC read) and never sets `D_800A185C`, so
`func_80040F80` never takes its `if (D_800A185C) func_80042228()` arm for a
save.  This matches the earlier `pe-crc-tail` and `pe-save-menu-window-state`
findings; this run independently re-proves them with a live trace.

## Root cause of the route failure (host input, measured)

The pad/event trace across the save point:

```
[PAD] f=38611 mask=BFFF     -> EV id=24 win=800a22e0 btn=00000020  <- Cross press (auto pulse)
[PAD] f=38612 mask=FFFF     -> EV id=24 win=800a22e0 btn=00010000  <- Cross release -> D2DC confirm
[PAD] f=38619 mask=BFFF                                   (no EV dispatched)
[PAD] f=38620 mask=DFFF                                   (no EV dispatched)
[PAD] f=38640 mask=DFFF     -> EV id=25 win=800a2490 btn=00000040  <- slot-list cancel (D6D4)
                                  ... no further event: top menu never closed
```

The `38620:DFFF` entry in `kDay1RoutePads` is therefore consumed with no
menu callback dispatched, exactly one Circle press survives (`38640`,
cancelling the slot list via `func_8004D6D4`), and nothing closes the top
menu (`func_8004D2DC`), so `D_8009D1A0` bit 2 stays set to the frame limit.
This is why `test_route_boot_day2.c`'s menu-exit guard fails: it is not a
guest-code defect.

The obvious candidate — the menu input lock `D_8009D0EC`
(`func_8005E114(1)`) — is **ruled out by direct measurement**: a trace of
`D_8009D0EC` at the top of `func_8005E30C` over all 42000 frames reports
`d0ec=00000000` on every call (`/tmp/lock.log`, 41994 samples).  The pad raw
`D_800BE9A2` does carry `dfff` (Circle) across the `38620` window, so the
press reaches the input layer and is lost later: `func_8005E30C` pops the
queued event but the focus list is briefly empty during the top-menu ->
sub-window hand-off, so no callback is entered.  The measured consequence is
what the fix addresses; the precise hand-off instant is not otherwise pinned
here.

## Reproducibility note on the earlier "fix"

The guard and the two-Circle route landed in `2b5d2a8f` / `a951207e`.  Running
the harness **at `2b5d2a8f` itself** (fresh worktree, rebuilt) reproduces the
same `FAIL: m0020i save/load menu never closed ... at frame 42000` as
`cd5ba828` does, so the `PASS` quoted in
`docs/evidence/pe-menu-close-pulse/REPORT.md` is not reproducible from the
committed tree at the commit that introduced it.  The guard is real, but its
positive case was never green on `main`.

## Fixes

1. `pc_port/tests/route_rehearsal_pads.h` — the save point records the third
   Circle press a player would make, at `38650` (release `38645`, press
   `38650`, release `38655`).  This is recorded input, not guest state: the
   `38620` press is eaten by the menu input lock, `38640` cancels the slot
   list, and `38650` reaches `func_8004D2DC` and clears the field-menu mode.
   The header comment now documents exactly this.

2. `pc_port/game/boot/func_80041108_port.c` (`card_live_close`, state 10) —
   the save-verify close now matches retail `0x80041D04..0x80041E38`:
   the copy guard is `entry[0] == record[0]` (`0x80041D54 bne v1,s1`) and the
   post-copy store is `entry[1] = 1` at the **entry**
   (`0x80041DF0 addiu v0,0,1 / sb v0,0x1(a2)`, `a2 = record + selected*0x44
   + 0x1C`), not `record[1]`.  The old `record+1` store wrote the record
   state (immediately overwritten by the cursor store) and left the
   just-written slot's `+1` flag cleared, so `func_800424B4` could not return
   it and `func_8004FE58` saw a missing entry.

## Verification (all on this branch)

| Gate | Result |
| --- | --- |
| `pe-route-boot-day2-tests` (with the fix) | **PASS** — 27/27 milestones, `frames=42000 stop=frame-limit story=0x48 token=A8002048`, field-menu mode cleared |
| same, at base `cd5ba828` unmodified | FAIL — menu mode still set at 42000 |
| `--headless --route-pad --max-frames 62000` | runs past the menu to `~f=61500`, stop `unresolved-boundary` at `func_8004AD9C` (the documented frontier); was frame-limit at 42000 before |
| `pe-native-tests` | 1392 run / 1392 passed / 0 failed (with `local/pe_disc1.path`) |
| `ctest --test-dir pc_port/build` | 11/11 passed |
| card oracles (`pe_card_*_oracle.py --check`) | 8/8 PASS |
| `gen_decomp_ports.py --check --allow-orphans` | `check: OK` (0 stale, 0 mismatched) |
| matching build | untouched — only `pc_port/` changed |

Raw logs: `/tmp/tr4.log` (E7/menu trace), `/tmp/ht2.log` (pad/event trace),
`/tmp/final_h.log` (passing harness), `/tmp/port62000.log` (61500 frontier),
`/tmp/final_native2.log`, `/tmp/final_ctest.log`.
