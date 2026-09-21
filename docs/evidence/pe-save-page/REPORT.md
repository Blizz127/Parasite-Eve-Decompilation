# PE-SAVE-PAGE — native port of `func_80043DA4` command 5 (file save/load page)

Worktree `/tmp/pe-agent-4ad9c` (branch `agent/4ad9c-savepage`) from `160137a4`.

## What was done

`func_80043DA4` (the field main-menu handler) previously stopped at an
unresolved boundary for command 5 and printed
`Bootstrap_ReturnVoid("func_8004AD9C", "func_80043DA4")`.  Command 5 is the
file save/load page.  It is now native.

16 non-matching retail functions were transcribed instruction-for-instruction
from their disassembly and are wired into the two guest-callback dispatch
switches:

| function | unit | size | role |
| --- | --- | --- | --- |
| `func_8004AD9C` | 37CD0.s | 0x80 | page entry: window id 0x20, +0x2C handler, +0x30 draw, `func_800647D0(node,4)` |
| `func_8004AE1C` | 37CD0.s | 0x120 | page input handler; confirm jump table `jtbl_80011034` (6 entries: 0..3 sub-pages, 4 and 5 both the status block) |
| `func_8004AF3C` | 37CD0.s | 0x68 | slot page A (window 0x21) |
| `func_8004AFA4` | 37CD0.s | 0x98 | slot page A handler |
| `func_8004B03C` | 37CD0.s | 0x68 | slot page B (window 0x23) |
| `func_8004B0A4` | 37CD0.s | 0x98 | slot page B handler |
| `func_8004B13C` | 37CD0.s | 0xd8 | slot-detail page (window 0x2E + nested list 0x31) |
| `func_8004B214` | 37CD0.s | 0x180 | slot-detail draw callback |
| `func_8004B394` | 37CD0.s | 0x1a0 | slot-detail input handler (packed-meter byte lanes) |
| `func_8004B584` | 3BD84.s | 0x58 | modal confirm page (window 0x38, modal=1) |
| `func_8004B650` | 3BD84.s | 0xbc | modal page input handler |
| `func_8005D994` | 4E194.s | 0xf8 | status/name-entry screen builder |
| `func_8005247C` | 42664.s | 0x54 | record cursor copy |
| `func_80050C70` | 41470.s | 0x44 | list renderer installed by 0x21 |
| `func_80050CB4` | 41470.s | 0x44 | list renderer installed by 0x23 |

The five one-line list-draw wrappers (`func_8004FF30/58/80`, `func_8004B534`,
`func_8004B55C`) are matched C leaves in `src/`; they are transcribed in
`func_8004AD9C_port.c` with the second argument spelled as its guest address
(`func_800638D8(slot, 0x80050C50u)`).  The generator would emit the bare
function designator there and lose the address conversion, so these five are
not in the `game/decomp` generated set; `src/` is still the authority for the
slot argument and target.

## Verification

```
# pc_port, in the worktree (needs local/pe_disc1.path + build/pe_card1.mcr,
# both git-ignored retail fixtures)
cmake -S pc_port -B pc_port/build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build pc_port/build -j24
ctest --test-dir pc_port/build
# 100% tests passed, 0 tests failed out of 11
#   native-tests            1394 run, 1394 passed, 0 failed, 0 skipped
#   route-boot-day2         PASS 27/27 milestones, frames=42000 stop=frame-limit
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans
# check: OK   (targets 272, stale boundaries 0, mismatched 0)
```

New regression tests (in `pc_port/tests/test_native.c`):

* `SAVEPAGE_8004AD9C_entry_callbacks_owner_and_rows` — the entry's owner at
  +4, handler at +0x2C, absence of a +0x30 draw on the window, the list draw
  callback `func_8004FF30` at +0x30, the four rows from `func_800647D0`, and
  the `selected == -1` guard through the jump table.
* `SAVEPAGE_8004AE1C_confirm_builds_the_four_slot_pages` — confirm cases 0..3
  build the 0x21 / 0x23 / 0x2E (+nested 0x31, with both sibling links and the
  +0x28 flag) / 0x38 windows with their exact handler and draw callbacks; the
  0x38 page republishes the alarm timer at `0x8009CFE4`; cancel unlinks the
  0x21 page.

The change touches only `pc_port/`, which is not an input to
`scripts/build_us.sh`, so the retail byte-exact image is unaffected.  Confirmed
by running the full retail chain in this worktree (split -> build -> verify):

```
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-4ad9c && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-4ad9c && bash scripts/verify_us.sh'
# OK    original EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
#       cand SHA-1       452fb033f2eaa4b18aa20a5bca60b8125af3a37b
#       RESULT: EXACT MATCH
# VERIFY_US=PASS   (709 c + 321 asm + 2 rodata spans, all 709 packed C spans equal retail)
```

All builds were run inside the worktree `/tmp/pe-agent-4ad9c` only; the shared
main checkout was never built.

## Finding: command 5 is not on the recorded Day-1 route

Instrumenting `func_80043DA4` and the new page functions with the retail
fixtures in place shows the recorded route never selects command 5: over the
full 42000-frame run `func_80043DA4` handles exactly one confirm, with
`command == 0`.  `func_8004AD9C` and every sub-page function are invoked zero
times.  The route's m0020i "save/load menu" is the memory-card flow
(`func_8004D6D4` / `func_8004D2DC`), not this file menu.

Consequences:

* Porting command 5 is a real fidelity fix (the boundary is gone and the page
  is native), but it is **not** what gates the `m0020i` `local[4]==3` advance;
  the route outcome is byte-identical with and without this change.
* The `route-boot-day2` guard failure that appears in a fresh worktree is an
  environment artefact: the test needs the git-ignored `build/pe_card1.mcr`.
  Without it the card model is blank, the slot list retries and the
  field-menu mode bit never clears.  With the fixture, baseline and this
  change both PASS.

## Remaining boundaries in this page tree

`func_8004B5DC` (the 0x38 modal draw callback) still routes through the loud
`PE_MenuDrawCallback` boundary, and `func_80050438` (the 0x21/0x23 slot cell
renderer) plus the matched leaf `func_80050C50` are reached through
`game/decomp` boundary/port TUs.  Completing them needs the unported
`0x8005E` text/font subsystem (`func_8005FCAC`, `func_8005ED18`,
`func_8005F5B8`, `func_8005FB74`, ...).
