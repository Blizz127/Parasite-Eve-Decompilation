# Save-menu input: func_8004D978 + func_8004D6D4 ported; input-callback wall cleared

Branch `agent/menu-input`, worktree `/tmp/pe-agent-menuinput`, base `85d1e76d`
("docs: save menu renders; route stop is now func_8004D6D4 (menu input)").

## What the wall was

The Day-1 `--route-pad` autopilot renders the real "Select File to Save" menu
(present `.mcr` card) and then stopped at
`[STUB:BOOTSTRAP_RET] PE_MenuInputCallback` for `func_8004D6D4` — the slot-list
window's `+0x2C` input/handler, which the `menu_callback` dispatcher in
`game/boot/func_80063E0C_port.c` did not have a case for. All 22 of its callees
were already ported except `func_8004D978` (24 words).

## Implemented (translated from retail bytes, not matching leaves)

`pc_port/game/boot/func_8004D4C4_port.c`:

| function | retail span | what |
| --- | --- | --- |
| `func_8004D978(value)` | `3DCC4.s` 0x8004D978..0x8004D9D8 (24w) | build the prompt/notice window: a (0x27) node under the `func_80062CC4()` parent with the `func_8004DA04` draw callback at `+0x30` and the `func_8004DA9C` input callback at `+0x2C`, then publish `value` to `D_8009D000` (`gp+0x290`) |
| `func_8004D6D4(window, event)` | `3DCC4.s` 0x8004D6D4..0x8004D978 (169w) | the slot-list input/handler |

`func_8004D6D4` semantics, faithfully:

- `event & 0x40` — close/cancel: `func_80062F3C(0x3F)`, `func_80062F1C(window)`,
  `func_80042A10()`, `func_80052634()`, return 1.
- `event & 0x10000` — confirm: `selected = func_8006346C(list)` stored to
  `D_8009CF48`; negative → unable (`func_800526C4`, return 1). Otherwise look the
  slot entry up with `func_800424B4(D_8009CF44, selected)`; null, or
  `D_8009CF50 == 0 && entry[0] == 2`, → unable.
- `D_8009CF4C = entry[0]`; `D_8009CF50 == 0` → the 0x46 unable notice
  (`func_80062F3C(0x1F)`, `func_8004D978(0x46)`, `func_80042B50(0x8005051C)`).
- Otherwise `func_8003FFAC(D_8009CFF8)`. If `entry[0] == 2` (occupied): with
  fewer than 0xF files, the 0x45 load/overwrite prompt (`func_8004D978(0x45)`,
  `func_80042B50(0x800504F4)`); else the `func_8004CE28(idx+0x47,
  D_8009CF50+0x42)` notice.
- Generic continue arm: build the (0x29) sub-list (`func_80062D2C`/
  `func_8006322C`), install the `0x80044E14`/`0x80044E98`/`0x8004F950`
  callbacks, measure the label with `func_8005F1A0`, lay out
  `+0x34/+0x38/+0x18` (row and child), publish the `0x80050544` scroll callback
  to `D_8009CFA8`.

Wiring:

- `func_80063E0C_port.c` `menu_callback`: `case 0x8004D6D4u -> func_8004D6D4`.
- `func_8005C498_port.c` `func_80042B6C` (the `func_80042B50` delayed-callback
  consumer) gained `0x800504F4 -> func_800504F4()` and
  `0x8005051C -> func_8005051C()`, which previously fell to
  `[MENU] Unported delayed callback` + `menu_boundary` (a stop).
- `pe_port_compat.h`: prototypes for `func_8004D6D4`, `func_8004D978`,
  `func_800504F4`, `func_8005051C`; local externs for the matching leaves
  `func_8003FFAC` and `func_80042964`.

## Verified

Commands (worktree root `/tmp/pe-agent-menuinput`):

```text
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build -j
./pc_port/build/pe-native-tests          -> 1384 run / 1384 passed / 0 failed / 0 skipped
ctest                                    -> 11/11 passed
card oracles (status/operation/frame/driver) -> PASS (4096/8192/1280/96)
./pc_port/build/parasite-eve-port --disc-image "$(cat local/pe_disc1.path)" \
    --headless --route-pad --max-frames 42000 --boundary-report --trace /tmp/pe.log
```

Live route, exact before/after (real Disc 1, present `.mcr`):

```text
BEFORE (85d1e76d): [ROUTE] frame=38500 story=0x48
  [STUB:BOOTSTRAP_RET] PE_MenuInputCallback (first invocation)
  [HOST] stop_reason=unresolved-boundary

AFTER:              [ROUTE] frame=42000 story=0x48
  [STUB:BOOTSTRAP_RET] func_80042020 (first invocation)   <- decomp boundary, does NOT stop
  [HOST] stop_reason=frame-limit
```

The `PE_MenuInputCallback` STOP is gone; the recorded route now runs to the
42000-frame limit. The save-menu confirm path executes and reaches the
save-write handler `func_80042020`.

## Remaining frontier (precise, not faked)

The route still loops at story `0x48`: the slot-list confirm schedules the
save-write handler `func_800504F4 -> func_80042020`, and `func_80042020` is a
loud decomp boundary because it and its card-write callees are unported
nonmatching functions:

```text
func_80042020  0x150 (84w)   save-write entry (calls func_80042798, func_8005C25C,
                             func_80071A84, func_80040B80)
func_80042170  0x0B8 (46w)   load entry (calls func_80042798, func_80071A24)
func_8005C25C  0x118 (70w)   MISSING
func_80040B80  0x400 (256w)  MISSING
```

`func_80071A84` is the SDK formatter, already implemented host-side as
`PE_FormatterFrame`, but with an explicit caller-stack ABI that a new caller
must reproduce. Completing the save therefore means porting
`func_80042020`/`func_80042170` plus `func_8005C25C`/`func_80040B80` (and any
of their callees that are still missing) against the libcard file API — a
separate, larger task. The boundary is loud and does not silently fake a save.

## Oracle integration (honest)

The card-operation oracles replay recorded original executions. Adding the
`menu_callback` `0x8004D6D4` case and the `func_80042B6C` delayed-callback cases
does not change their recorded paths: all four card oracles still PASS, and the
`pe_card_status` oracle still models the empty-slot kernel.
