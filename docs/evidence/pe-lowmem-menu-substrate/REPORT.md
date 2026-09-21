# Low-memory kernel/menu substrate + slot-list constructors

Branch `agent/lowmem-kernel`, worktree `/tmp/pe-agent-lowmem`, base `5cee639e`.

## The "low memory" was a misdiagnosis — here is the evidence

The report that motivated this task said the two `func_80041108` state-2
continuations "read PS1 low memory (address 0 / 0x150)". That is not what the
menu code does. A full disassembly scan of the retail EXE (`build/disc1.elf`,
175,797 lines) finds **exactly six** zero-based (absolute low-address) memory
accesses in the whole game:

```
80072810:  lw v1,340(zero)   # [0x154]
80072820:  lw s0,336(zero)   # [0x150]
800728b8:  lw v1,340(zero)
800728c8:  lw s0,336(zero)
80072990:  lw v1,340(zero)
800729a0:  lw s0,336(zero)
```

All six are inside `func_800727B4` (libcard `firstfile`). `[0x150]` and
`[0x154]` are the BIOS libcard file-table pointer/count that `firstfile`
iterates with a 20-byte stride. `func_8004D4C4`, `func_8004D298`, and the menu
draw/input callbacks touch **no** low memory — their globals are ordinary
`$gp`-relative data.

The port already replaces the libcard file API host-side (`pe_libcard.c`), so
the live route never executes the retail `firstfile` body. The only place the
low-memory globals matter is the original-code oracle fixture, and
`pe_card_operation_oracle.py` already documents/model them as the empty file
table (`r[0x9D154..] = 0` and a `firstfile_hook`). **No new low-memory model is
required by the port or the live route.** The real blocker was the unported
menu callbacks.

`gp` resolves to `0x8009CD70` (proved by `func_80062A34`'s `lw v1,996(gp)` =
`D_8009D154`), so:
- `0x1D4(gp)` = `D_8009CF44` (constructor stores the record index)
- `0x1E0(gp)` = `D_8009CF50` (unable path reads the string base)

Both are already used elsewhere in the port, independently confirming the base.

## Implemented (translated from retail bytes, not matching leaves)

`pc_port/game/boot/func_8004D4C4_port.c`:

| function | retail span | what |
| --- | --- | --- |
| `func_8004D4C4(idx, items)` | `3DCC4.s` 0x8004D4C4 (66w) | continue-path slot-list constructor: `(2,0x24)` parent, `(idx+0x25)` list/row nodes, callbacks `8004D6D4`/`8004FEEC`/`8004FE58`, item layout, cursor restore/free, `0x3F` prompt node with `func_8004D690`, publishes idx to `D_8009CF44`; returns `func_80063428`'s result |
| `func_8004D298(index)` | `3DA98.s` 0x8004D298 (17w) | unable path: build the `(index+0x47)` notice window when no `(1,0x28)` node exists |
| `func_8004D690(prompt)` | `3DCC4.s` 0x8004D690 (17w) | prompt-row draw callback |
| `func_800424B4(card,item)` | `32CB4.s` 0x800424B4 (33w) | slot-entry lookup (card<2, item<record[+2], entry[+0x1D] != 0) → entry+0x1C |
| `func_8004FEEC(list)` | `401A0.s` 0x8004FEEC (17w) | list draw callback: publish node, draw with the `func_800434C0` row callback, flush |
| `func_8004FE58(item)` | `401A0.s` 0x8004FE58 (37w) | slot row state predicate |

`pc_port/game/boot/func_800434C0_port.c`:

| function | retail span | what |
| --- | --- | --- |
| `func_800434C0(item)` | `33CC0.s` 0x800434C0 (189w) | slot-row draw callback: occupied / empty / new-file layouts (name, time, level, screenshot, icons), exactly the retail `func_8005E8A4`/`func_8005EB64`/`func_8005F27C`/… sequence |
| `func_8005DD8C(index)` | `4E58C.s` 0x8005DD8C (31w) | `D_800A8028` string-table lookup it feeds to `func_8005F27C` |

Wiring: `func_80041108` state 2 now calls the constructors on the live path
(`record+5` gets the low byte of `func_8004D4C4`'s return, as the retail delay
slot at 0x80041594 does); `menu_draw_callback` gained `0x8004D690`,
`0x8004FEEC`, `0x8004FE58`, `0x800434C0`; `func_800634D4` gained the
`0x8004FE58` cell predicate.

## Verified

- **Live route** (`--headless --route-pad --max-frames 42000`, real Disc 1):
  the wall moved two steps —
  `func_8004D4C4` → `func_800434C0` (`PE_MenuDrawCallback`) → now
  **`PE_MenuInputCallback` for `func_8004D6D4`** (the slot-list input/handler).
- **`pe-native-tests` 1384 run / 1384 passed / 0 failed / 0 skipped.**
- Matching rebuild untouched (only `pc_port/` changed).

## Oracle integration (honest)

The card-operation oracles replay a *recorded original execution* whose menu
node pool (`func_80062F9C`'s free list at `D_8009D158`) was never captured; the
recorded runs stopped at `0x8004D4C4`, so their pool is empty. Running the
constructor against an empty pool dereferences a null free-list head
(`PE_LoadU32(0)`), which is exactly the fault the earlier attempt hit. The live
boot path *does* run `func_80062F9C`, so the live route has a real pool and
takes the real constructor.

To keep the oracle honest rather than faking a pool, the port keeps the
**recorded named boundary** when the menu pool is absent or when a supplied
oracle frame is being replayed:
`if (incoming || PE_LoadU32(0x8009D158u) == 0u) operation_boundary(...)`.
This preserves the recorded oracle hashes and boundary payloads; the live path
is unaffected. A future oracle regeneration should run `func_80062F9C` in the
fixture and move the STOPS past these callbacks, which would then cover the
host implementations directly.

## Next frontier

`func_8004D6D4` (slot-list input/handler, `3DCC4.s` 0x8004D6D4, 169 words).
All 22 of its callees are already ported except `func_8004D978` (`345E8.s`,
24 words), so it is port-able. After it, the remaining menus (name entry,
format, load) follow the same pattern.
