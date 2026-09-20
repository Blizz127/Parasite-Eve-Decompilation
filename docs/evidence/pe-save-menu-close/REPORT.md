# Day-1 save menu now CLOSES: root cause was the autopilot, not the port

Branch `agent/menu-close`, worktree `/tmp/pe-agent-menuclose`, base `4db80519`
(707 matching leaves).  Authority: retail Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The standing blocker "the Day-1 save menu never closes / the route parks at
story `0x48`" is **not a port bug**.  The retail close path was already
correct and ported; what kept the menu open was the `--route-pad` autopilot
re-opening it with its automatic periodic Cross.

## Root cause

`pc_port/src/port_main.c`'s `RoutePadSource` applies an automatic Cross pulse
on every frame where `g_frame % g_route_pad.period == 3`.  On the recorded
route the Day-1 save point is reached at frame 38611, so:

```text
38611  auto-Cross  -> opens the save menu
38612  [D2DC] win=800A22E0 event=00010000   (slot-list confirm)
38619  auto-Cross  -> confirms/saves again
38627  auto-Cross  -> ...
```

The pulse never stops, so the menu is re-confirmed forever and the field
menu mode is never left.  The port's own close path was never at fault.

### Two further facts that the earlier investigations got wrong

1. **The menu exit needs a Circle PRESS, not a release.**  `func_8005E30C`
   (`pc_port/game/boot/func_80063E0C_port.c:226`) translates only
   `type==4 && buttons&32` (Cross release -> `0x10000`); every other type-4
   event hits `else return;` and is dropped.  Circle arrives as a **type-1
   press** with `buttons=0x40`, which is forwarded verbatim.  Probes that
   held Triangle/Circle/Cross/Square, or that injected a Circle release,
   could therefore never work.
2. **A single Circle press is not enough.**  The first `0x40` is consumed by
   the slot-list window (`node 800A2520`, callback `func_8004D6D4`), which
   tears itself down and returns handled.  Only a **second** press on the
   next frame reaches the file menu (`win 800A22E0`, callback
   `func_8004D2DC`).

## The retail close path (unchanged port code, now actually reached)

```c
/* func_80015AF0_port.c: func_8004D2DC, event&0x40 arm */
for (i=0;i<4;i++) func_80062F1C(func_80062A34(1,ids[i]));  /* 37,38,36,19 */
func_8005C1EC(0);
func_800512AC(9,0);        /* -> D_8009D010 = UINT32_MAX */
PE_StoreU32(0x8009CFF8u,0);
func_80052634();
```

Measured (temporary probes, removed before commit):

```text
[D2DC]  f=38612 win=800A22E0 event=00010000   <- slot-list confirm
[D2DC]  f=38640 win=800A22E0 event=00000040   <- Circle press #2
[C498]  f=38640 MENU-RESULT=FFFFFFFF d034=0
[FMODE] f=38640 field-menu-mode EXIT res=FFFFFFFF
```

`func_8005C498` returns `UINT32_MAX` and `PE_FieldMenuFrame` clears the field
menu mode.  No game-side change was needed.

## Fix

1. `pc_port/tests/route_rehearsal_pads.h` — the recorded route now carries the
   two Circle presses that a human would make:

   ```text
   ...38090:FFEF,38610:FFFF,38620:DFFF,38630:FFFF,38640:DFFF,38650:FFFF,42000:FFEF...
   ```

2. `pc_port/src/port_main.c` — a **second, independent** periodic-Cross
   suppression window `g_pulse2_begin=38620, g_pulse2_end=42000`
   (`PE_ROUTE_PULSE2_BEGIN` / `PE_ROUTE_PULSE2_END` override it for probing).
   The existing window is `[PE_ROUTE_PULSE_END, PE_ROUTE_PULSE_RESUME)` and
   must stay; the old single window could not express both.

3. `pc_port/game/boot/func_8005C498_port.c` — `func_8004DCA4` mode!=0 now
   implements the retail `func_80054294()==0 -> func_8005C488()` close leg
   (`asm/disc1/3E4A4.s` `0x8004DCE4`, `beqz $v0,.L8004DD08`).  This is **not**
   exercised by the route (`func_8004DCA4` and `func_80016F10` are entered 0
   times over 39000 frames); it is kept because it is strictly more faithful
   than the blanket boundary it replaces.  The old comment claiming this
   branch was "why the save menu never closed" was **wrong** and is corrected.

## Discrimination (clean binary, no probes)

Both runs use only the committed env hooks, `--headless --route-pad
--max-frames 70000`.  `PE_ROUTE_PULSE2_*` moves the new window, so disabling
it reproduces the pre-fix autopilot exactly.

| run | `PE_ROUTE_PULSE2_BEGIN/END` | last `[ROUTE] frame=` | stop_reason |
|-----|------------------------------|----------------------|-------------|
| A (fix)      | default (38620/42000) | **42000** | unresolved-boundary |
| B (pre-fix)  | `0`/`0`               | **38500** | unresolved-boundary |

Raw logs: `A_fix_70k.out`, `B_nofix.out` in this directory.  With the fix the
route leaves the save point and reaches the frame-42000 card-boundary
frontier; without it the route is still inside the menu churn at frame 38500.

## Next frontier (measured, not fixed)

The route now stops deterministically at a genuine card boundary, reported by
a temporary probe in `operation_boundary`:

```text
[CARDBND] caller=func_80040F80 target=80072774 a0=00000000 a1=0 a2=0 a3=0 known=1
[STUB:BOOTSTRAP_RET] card operation unresolved call (first invocation)
```

`func_80040F80` (`pc_port/game/boot/func_80041108_port.c:558`) stops at
`func_80072774` instead of running the retail
`0x80040FC8 jal func_80072774; 0x80040FD0 record[+0xC] = -1` and falling
through.  `func_80072774` **is** already implemented natively
(`pc_port/platform/pe_libcard.c:673`) and covered by the card-status oracles.
Porting the call was tried and reverted: it breaks
`DAY1_card_cleanup` (1392/1392 -> 1391/1392) because
`retail_card_cleanup_cases.h` was generated with `0x80072774` in the shared
stop set (`pc_port/tools/pe_card_operation_oracle.py:6`).  Doing it correctly
means removing `0x80072774` from that stop set and regenerating the
card-operation / operation-frame / cleanup / driver oracle headers together,
plus the native boundary sites that share the callee
(`PE_CardOperationFrame`, `PE_CardCleanupFrame`, `func_80040F80`) and
`pc_port/tests/test_exit_menu_input.h:22`, which asserts that target.

Beyond that boundary, `m0020i` itself does not advance: Aya is live and
walkable there (position moves `(537,-19) -> (94,173) -> (-166,-489) ->
(-546,231)` over frames 22000..34000, `D1A0` toggles `0x4040`/`0x4044`), but
the recorded pads park her near `(-9,122)` from frame ~44000 and the M0020I
key pickup never fires - a temporary probe on `func_8004F490` /
`func_80015BAC` (`field_item_pickup_port.c`, the opcode-A7 award path) logged
**0** invocations over 45000 frames.  Leaving `m0020i` needs either newly
authored route input or the unported field contact pass described in
`docs/evidence/pe-sew-day1-corridor/REPORT.md` (SEW18).

## Verification

```text
./pc_port/build/pe-native-tests  -> 1392 run / 1392 passed / 0 failed
ctest (pc_port/build)            -> 11/11 passed
   incl. route-boot-day2-control-flow (consumes kDay1RoutePads) and
         decomp-port-reproducibility
```

The change set touches `pc_port/` only - no `src/`, `configs/`, or
`asm/` change - so the matching build is unaffected
(SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` for both discs).

## Reproduction

```sh
cd /tmp/pe-agent-menuclose
cmake --build pc_port/build -j"$(nproc)"
IMG="$(cat /home/blizz/dev/Parasite-Eve-Decompilation/local/pe_disc1.path)"
# fix active
./pc_port/build/parasite-eve-port --disc-image "$IMG" --headless --route-pad \
    --max-frames 70000 --boundary-report
# pre-fix autopilot (route never leaves the menu)
PE_ROUTE_PULSE2_BEGIN=0 PE_ROUTE_PULSE2_END=0 \
./pc_port/build/parasite-eve-port --disc-image "$IMG" --headless --route-pad \
    --max-frames 70000 --boundary-report
```
