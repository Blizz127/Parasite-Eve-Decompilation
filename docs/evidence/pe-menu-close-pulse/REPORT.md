# m0020i save/load menu exit: the autopilot Cross pulse re-opened the menu

> Companion to `docs/evidence/pe-menu-close-parity/REPORT.md` (harness parity,
> regression guard, and the open suppression window).  This file is the
> frame-exact root-cause trace behind the fix that landed as `a951207e`.
> The raw logs referenced below were produced on branch
> `agent/menu-exit-action`; the excerpts are inline here.

**Status: confirmed root cause, fixed, regression-guarded.**

## Question

After the Day-1 (m0020i) save completes, the `--route-pad` autopilot never
leaves the save/load menu. `func_8004D2DC`'s retail close path was already
proven faithful, so the question was why the close never happens on the route.

## What actually happens (instrumented, frames are `g_frame` present-hook ticks)

Menu callbacks were logged at entry for the whole save-menu segment:

```
[M2DC] f=38612 n=1 win=800A22E0 event=00010000 sel=0     <- top menu confirmed into "save"
[M6D4] f=38620 n=1 win=800A2490 event=00010000           <- slot list, confirm
[M6D4] f=38621 n=2 win=800A2490 event=00000040           <- slot list cancel (Circle)
[M2DC] f=38640 n=2 win=800A22E0 event=00000040 sel=0     <- TOP MENU CLOSE BRANCH
```

`func_8004D2DC` with `event & 0x40` runs its unmodified close path:
`func_8005C1EC(0)` -> `func_800512AC(9,0)` -> `D_8009D010 = UINT32_MAX` ->
`func_8005C498` returns nonzero -> `PE_FieldMenuFrame` clears the field-menu
mode. After f=38640 neither callback is ever entered again (0 calls through
f=50000).

The two Circle presses that drive it are the recorded sequence at 38620 and
38640 (`38620:DFFF,38622:FFFF,38640:DFFF,38642:FFFF`).

## Root cause

`port_main.c`'s `RoutePadSource` mashes Cross every 8 frames
(`g_frame % period == 3` -> `mask &= g_route_pad.pulse`). That pulse is
"+ confirm", and it is the only input the recorded sequence supplies after
38610 (`38610:FFFF`, then idle until `42000:FFEF`). With the pulse running:

```
[M2DC] f=38612 n=1 event=00010000   <- save
[M2DC] f=38628 n=2 event=00010000   <- re-confirm "save"
[M2DC] f=38644 n=3 event=00010000
[M2DC] f=38668 n=4 event=00010000
[M6D4] 98 calls, last at f=39396
```

The top menu is re-confirmed every 8 frames, each confirm re-creates a slot
list, and the menu never closes. The port's *close code* was correct all
along; the *route input* was fighting it.

This is a host-input scheduling bug, not a guest-translation bug: nothing in
`pc_port/game/**` changed.

## Fix

1. `pc_port/include/pe_route_pad.h` — `PeRoutePad_PulseAllowed(frame, period,
   off1_begin, off1_end, off2_begin, off2_end)` is now the single source of
   truth for the suppression rule, with the two recorded windows named:
   * `[33620, 35300)` — the m0004i segment (pre-existing; the port and the
     harness hard-coded it separately and could drift).
   * `[38620, INT_MAX)` — the m0020i save/load menu exit.
2. `pc_port/src/port_main.c` — uses the shared rule; the two windows are
   `PE_ROUTE_PULSE_OFF{1,2}_{BEGIN,END}` and overridable from the environment
   (`PE_ROUTE_PULSE_OFF1_BEGIN`, `..._OFF1_END`, `..._OFF2_BEGIN`,
   `..._OFF2_END`).
3. `pc_port/tests/route_rehearsal_pads.h` — records the two Circle
   press/release pairs that leave the menu.
4. `pc_port/tests/test_route_boot_day2.c` — same shared rule, plus a
   regression guard.

## Regression guard (non-vacuous)

`PE_FieldMenuFrame` sets `D_8009D1A0` bit 2 while the field menu is up and
clears it when the menu result is nonzero. The harness samples that bit for
frames >= 38600 and fails if it is still set at/after 38660.

* with the fix: `d1a0=00004040` at f=39000 (bit 2 clear) -> PASS
* with window 2 disabled (`PE_ROUTE_PULSE_OFF2_BEGIN=0 PE_ROUTE_PULSE_OFF2_END=0`
  in the port; `0, 0` in the harness): `d1a0=00004044` at f=39000 (bit 2 set)
  -> `FAIL: m0020i save/load menu never closed (field-menu mode still set at
  frame 42000)`

Verified by temporarily passing `0, 0` for window 2 to
`PeRoutePad_PulseAllowed` in the harness: the test fails with exactly that
message, and passes again once restored.

## Evidence

* `trace-with-fix.log` — instrumented run: 2 `M2DC` calls (38612 confirm,
  38640 cancel), 2 `M6D4` calls, nothing after 38640.
* `trace-pulse-on-loop.log` — instrumented run with the pulse left on:
  4 `M2DC` confirms, 98 `M6D4` calls, menu never closes.
* `route-clean-fix.log` — the committed (uninstrumented) port, 39400 frames.

## Result

The route reaches m0020i at the same milestone and the menu now closes on the
retail cancel path. **The route still parks at token `A8002048` / story `0x48`
through frame 52000**, because the recorded sequence carries no input that
moves the field forward after the menu closes. That is the next, separate
blocker (m0020i field script), not the menu.

## m0020i after the menu — what the field is waiting on

With the menu closed the field is genuinely alive again, so the park is a
field-script condition, not a stalled port:

* **VM healthy.** `func_80017018` dispatches ~26 script opcodes per frame
  after the close (13,080 per 500 frames, measured 39000..42000).
* **Aya is controllable.** Her actor position changes with the recorded pads
  (f=39000 `x=FFF419E1 z=00710328` -> f=42500 `x=004B7B89 z=00ABACB5`).
* **Nothing requests a transition.** Room transitions go through opcode 0x31
  (`func_80017BB4`, which writes `D_8009D280`). Instrumenting the token shows
  m0020i is entered at **f=20439** and `D_8009D280` **never changes again**
  through f=52000 — no `op31` is executed.
* **The m0020i item-200 award never fires.** Instrumenting the opcode 0xA7
  dispatcher (`func_800194B0`) shows **0 invocations** over the whole run. Its
  call site is the block

  ```
  801A0AE0: OP09_alu subop 0x0B, dst=K[0], a=Aya.local[4], b=3   ; subop 0x0B = (a==b)
  801A0AF8: OP05 skip-if-zero -> 801A0CC4
  ...
  801A0B8C: op0D #=0x73
  801A0B98: op22 #=0x73
  801A0BA4: opA7 #=0xC8  local[6]        ; award item 200
  801A0BB4: OP09_alu subop 0x0B, K[0], local[6], 0
  801A0BCC: OP05 skip-if-zero -> 801A0C1C
  801A0BDC: opE8 #=0xC8                  ; pickup dialog
  ```

  Aya's `local[4]` sits at **1**, so the `local[4] == 3` gate never opens.
  (This matches SEW17's "M0020I awards item200 at 801A0BA4 through opcode A7,
  then sets global24 bit20" — the award is real but unreached.)
* **The positional triggers are looping.** Actors type 4/5/6 run `op77`
  rectangle tests each frame. Actor 5's rectangle reports a hit
  (`local[4]=1`) from f~39000 to f~43000 after the recorded Up input, and its
  condition chain (`op5E`/`op09`/`op11`/`opAA`/`op1C`/`op0A` at
  801A1F40..801A2034) never reaches a transition.

**Named blocker:** m0020i is parked on the field-script gate
`Aya.local[4] == 3` (0x801A0AE0 / 0x801A0AF8), which guards both the item-200
award and the rest of the room's event block; no recorded input in
`kDay1RoutePads` puts Aya into that region, and nothing else in the room
requests a room transition. The recorded sequence's post-menu pads
(`42000:FFEF` Up, `42400:FFFF`, then the 42713..45041 "raw battle pads") were
recorded for a segment the port does not reach; whether that segment is a
random encounter or the item approach is the next thing to pin down.
