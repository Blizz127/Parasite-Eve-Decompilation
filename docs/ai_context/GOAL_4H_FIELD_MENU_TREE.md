# Goal: close the field-menu tree (equipment / modal / close pages)

Time box: 4 hours. Deliverable: the native PC port (`pc_port/`); matching decomp
is the means. Recovered asm is worth zero to the port. No MIPS interpreter.
Nothing is committed, per repo convention.

## Objective

The previous increments translated the `0x8004AE1C` field-menu page handler, its
four sub-page constructors, the Items/Escape handlers and their draws, leaving six
**named** boundary arms in the two dispatchers:

| Symbol | File | Words |
| --- | --- | --- |
| `func_8004B214` | `asm/disc1/37CD0.s` | 96 |
| `func_8004B394` | `asm/disc1/37CD0.s` | 104 |
| `func_8004B650` | `asm/disc1/3BD84.s` | 47 |
| `func_80050438` | `asm/disc1/40B3C.s` | 33 |
| `func_8005D994` | `asm/disc1/4E194.s` | 62 |
| `func_8005247C` | `asm/disc1/42664.s` | 21 |

plus the `func_8004AE1C` jump-table case-4/5 arm (`PE_MenuInputClose`). Translate
them, wire the dispatchers, and prove the route is unaffected.

## Acceptance criteria

1. All six functions translated from the split asm into hand TUs under
   `pc_port/game/boot/`, one function per file, with the original address/word
   span quoted in each header.
2. Dispatchers route by the **correct slot**: window `+0x2C` = input
   (`func_80063E0C_port.c`), window/list `+0x30` = draw
   (`func_800638D8_port.c`).
3. `func_8004AE1C` case 4/5 calls `func_8005D994(index-4)` and the shared tail;
   no named boundary remains in that function.
4. Focused unit test covers every new arm and both dispatch slots, asserting no
   stop and no stub log.
5. Full native suite green; plain and reward-pilot routes unchanged.
6. Coverage/generator honesty: report whether `native_c` moved (it should not —
   these are asm-derived hand TUs, not matching-C leaves).
7. `git diff --check` clean; no game data; nothing committed.

## Outcome (2026-09-18, closed)

| Item | Result |
| --- | --- |
| `func_8004B214` | native — Equipment **window draw** (+0x30) |
| `func_8004B394` | native — Equipment **input** (+0x2C), packed-colour lane clamp |
| `func_8004B650` | native — Modal input (+0x2C) |
| `func_80050438` | native — Equipment list1 per-cell draw |
| `func_8005D994` | native — close page (view bytes, stat row, commit) |
| `func_8005247C` | native — commit: full heal (`record+0x1C` → `+0x0C`/`+0x0E`) |
| `func_8004AE1C` case 4/5 | native (`func_8005D994(index-4)` + shared tail) |
| Dispatch fix | `0x8004B214` moved from the input dispatcher to the draw dispatcher (window+0x30), an inverted slot left by the previous increment |
| Focused test | `DAY2_field_menu_input_4ae1c` PASS (incl. new parts 8–12) |
| Full suite | 1404 run / 1404 passed / 0 failed / 0 skipped |
| Plain route | unchanged (45/57, no unresolved boundary) |
| Pilot route | unchanged (57/57, PASS) |
| Coverage | `disc=1 funcs=372/979 native_c=258/979` unchanged (honest) |
| Generator | `gen_decomp_ports.py --verify`: 269 TUs, 0 drift |

Remaining named boundary in the tree: only `PE_MenuDrawCallback_8004B5DC` (the
modal window's own draw). Its callees `func_8005FCAC` (0x144 = 81 words) and
`func_8005ED18` (0x1B0 = 108 words) are still untranslated.

## Follow-up (2026-09-18): the residual above is closed — the tree is complete

The named residual was taken next and is now native:

| Item | Result |
| --- | --- |
| `func_8005FCAC` | native — signed-number printer (icon `0x52`/`0x89`, 2/3 digits, blank leading zeros) |
| `func_8005ED18` | native — 0x28-byte icon/sprite packet + ordering-table link |
| `func_8004B5DC` | native — the modal draw itself; `PE_MenuDrawCallback_8004B5DC` deleted |
| Focused test | `DAY2_field_menu_input_4ae1c` PASS, parts 7–14 |
| Full suite | 1404 run / 1404 passed / 0 failed / 0 skipped |
| Plain route | unchanged (`route_modal_plain.log`) |
| Pilot route | unchanged, 57/57 PASS (`route_modal_pilot.log`) |
| Coverage | `native_c=258/979` unchanged (asm-derived hand TUs, honest) |

**The field-menu tree now contains zero named boundaries.** The only arms left in
its dispatchers are the generic fallbacks (`PE_MenuInputCallback`,
`PE_MenuDrawCallback`) that fire for callbacks outside the class the port has
classified.

Two notes for the next worker:

- `func_8005ED18` routes the packet/OT pointers through the hardware KUSEG mirror
  (`< 0x200000 -> |0x80000000`) because retail tolerates a null packet pointer and
  the port's guest-range guard aborts. This matches the existing `menu_ram()`
  convention; it is not a behavioural change.
- The unit test needs the packet pool seeded *and* the ordering-table head zeroed
  before asserting exact link words, otherwise leftovers from earlier draws leak
  into the expectations.

### Gotcha worth remembering

The equipment/modal draws allocate from the menu packet pool at
`0x8009D100`/`0x8009D104` (`PE_MenuPacketAlloc`). A unit test that calls these
draws directly must seed the pool first (`menu4ae1c_draw_setup`); otherwise the
allocator returns small integers (20, 28) and a downstream packet writer
(`func_80077C84`) stores to an invalid guest address and aborts. The route always
has a seeded pool, so this is a test-harness requirement, not a port bug.
