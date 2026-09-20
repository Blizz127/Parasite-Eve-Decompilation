# Save-menu close: harness parity, a regression guard, and why the suppression window must stay open

Follow-up to `a951207e` ("Day-1 save menu now closes"). That commit landed the
autopilot Circle presses and the second Cross-suppression window in
`port_main.c`, but it did **not** update the boot -> Day-2 harness, which
carries its own copy of the suppression rule. Three consequences, all fixed
here.

## 1. The harness was no longer reproducing the port

`port_main.c` gained `!(g_frame>=38620 && g_frame<42000)`; the harness
(`pc_port/tests/test_route_boot_day2.c`) kept only the `[33620,35300)` window.
Because both apply the same recorded `kDay1RoutePads` table, the harness now
fed the 38620/38640 Circle presses **while still mashing Cross every 8
frames** -- exactly the combination that re-opens the menu. The harness
passed only because its stop condition is "reached m0020i", which is true
either way; it had silently stopped exercising the fix.

The rule now lives once, in `pe_route_pad.h`:

```c
static inline int PeRoutePad_PulseAllowed(int frame, int period,
                                          int off1_begin, int off1_end,
                                          int off2_begin, int off2_end);
#define PE_ROUTE_PULSE_OFF1_BEGIN 33620
#define PE_ROUTE_PULSE_OFF1_END   35300
#define PE_ROUTE_PULSE_OFF2_BEGIN 38620
#define PE_ROUTE_PULSE_OFF2_END   0x7FFFFFFF
```

The port defaults its four window scalars from those macros (the
`PE_ROUTE_PULSE_END`/`_RESUME`/`PE_ROUTE_PULSE2_BEGIN`/`_END` overrides are
unchanged) and the harness passes the macros directly.

## 2. A non-vacuous regression guard

`PE_FieldMenuFrame` sets `D_8009D1A0` bit 2 while the field menu is up and
clears it once the menu result is nonzero. The harness now samples that bit
for frames >= 38600 and fails if it is still set at/after 38660.

* fixed: `d1a0=00004040` at f=39000 (bit 2 clear) -> PASS
* negative test, `PeRoutePad_PulseAllowed(..., 0, 0)` in the harness:
  `FAIL: m0020i save/load menu never closed (field-menu mode still set at
  frame 42000)`

## 3. Ending the window at 42000 re-invents the bug

`a951207e` bounded the second window at 42000. The recorded tail from 42000
on carries explicit pads, so the automatic Cross is pure invention there, and
re-enabling it re-enters the file menu:

| second window | port stop |
| --- | --- |
| `[38620, 42000)` (as landed) | `unresolved-boundary` at **~f=44300**, `[STUB:BOOTSTRAP_RET] card operation unresolved call` |
| `[38620, INT_MAX)` (this change) | `unresolved-boundary` at **~f=61500**, `[STUB:BOOTSTRAP_RET] func_8004AD9C` |

Same room token (`A8002048`) and story (`0x48`) either way, so the open window
is both the more faithful input (no invented mashing) and the longer route.
`PE_ROUTE_PULSE2_END` still bounds it for probing.

The `func_8004AD9C` stop is the save/load menu's own command-5 page, listed as
unported in `pc_port/game/boot/func_800438EC_port.c` (`func_80043DA4`'s
`pages[]`), so the next port step from here is that page, not the menu close.

## Verification

* `pe-native-tests` 1392/1392
* full `ctest` 11/11 (including `route-boot-day2-control-flow`)
* `gen_decomp_ports.py --check --allow-orphans` OK
* port, real Disc 1, `--headless --route-pad --max-frames 70000`:
  `stop_reason=unresolved-boundary`, last route line f=61500

Host input scheduling and test code only; `pc_port/game/**` is untouched by
this change.
