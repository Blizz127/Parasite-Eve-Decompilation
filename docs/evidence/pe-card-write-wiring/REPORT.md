# Card-write wiring: libcard file API driven from the func_80041108 state machine

Branch `agent/card-write`, worktree `/tmp/pe-agent-cardwrite`, base `a598ddd0`
("699-leaf exact rebuild; save-write chain ported").  Authority: retail Disc1
EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `asm/disc1/307CC.s`
(`func_80041108`), `asm/disc1/621E4.s` (libcard veneers), `asm/disc1/32A64.s`
(`func_800424B4`).

## What was missing

The save-write chain (`func_80042020` → `func_80040B80`) assembled the
0x2000-byte block and armed the record, but `func_80041108`'s states 3..11 only
stopped at named boundaries: no `open`/`read`/`write`/`close`/`erase` was ever
issued, so the block was never written to the card.

## Implemented (hand-translated from the retail bytes)

1. **BIOS `B(0x45h)` `erase(filename)`** in `pc_port/platform/pe_libcard.c`
   (psx-spx: "Delete a file on target device", returns 1=okay / 0=failed).
   Releases the entry's data-block chain, clears the directory entry, persists
   the image.  Prototype in `pe_port_compat.h`.
2. **Live state machine** in `pc_port/game/boot/func_80041108_port.c`, gated on
   `PE_Card_IsPresent()`:
   - states 3/4/5/6/11: call the formatter (`PE_FormatterFrame`, retail
     `func_80071A84`) through the `0x801FF600` scratch caller frame, then run
     the retail continuation:
     - state 3 `@0x800416D8`: `open(name,1)` → state 8 header refresh
       (and `lseek(0x100,0)` when the entry read-marker is 1);
     - state 4 `@0x80041834`: `open(name,0x10200)` + `close` → state 6;
     - state 5 `@0x80041908`: `open(name,1)` → state 7 (load);
     - state 6 `@0x800419D4`: `open(name,2)` → state 9, buffer `D_8009EED0`,
       length 0x2000, count 0x1E;
     - state 11 `@0x80041E98`: `open(name,1)` + `close` + format + `erase(name)`
       → state 4.
   - states 7/8: read `min(len,0x400/0x80)` per iteration (retail
     `func_80072754`);
   - state 9: write the block in `0x400`-byte chunks (retail `func_80072764`),
     then close, state 3, `record[7]=0`, `D_800A1854=0`, `func_8004D9D8()`,
     `func_8004CC50(0x53,0)` (retail `@0x80041C0C`);
   - state 10: close + copy the loaded header fields into the slot entry +
     advance the walk cursor (retail `@0x80041D04`).
   Shared retry/failure tail (retail `0x80041754`/`.L80041798`/`.L80041FBC`/
   `.L80041FF8`): decrement the count, run `func_80040F80`, set state 12.
3. **Device-prefix fix** in `pe_card_name_from_guest`: the card stores the
   filename without the `buXX:` device specifier (psx-spx).  Retail's state-2
   match is `dirent[0..11] == [0x80092224]+6` = `"BASLUS-00662..."`; a stored
   name that kept the prefix never matched.
4. **Card-presence gating:** the no-card path keeps the original first
   unresolved call (the formatter), which is what the `DAY1_card_operation`
   and `DAY1_card_driver` oracles pin (`ResetTestState` sets the card absent).

## Verified

```text
./pc_port/build/pe-native-tests                  -> 1385 run / 1385 passed / 0 failed / 0 skipped
ctest                                             -> 11/11 passed
card oracles (status/operation/operation_frame/
  driver/record/confirmation) --check             -> PASS (4096/8192/1280/96/256/128)
```

Matching build untouched (no `src/`/`configs/` changes).

Live route, present `.mcr`, real Disc 1, headless `--route-pad`:

```text
[ROUTE] frame=... story=0x48 token=A8002048   (no unresolved-boundary stop)
[HOST] stop_reason=frame-limit
```

The save is actually written.  Dumping `build/pe_card1.mcr` (131072 bytes,
header `MC`) after the run:

```text
used entries: [(0, '0x51', b'BASLUS-00662000', 8192)]
```

i.e. directory entry 0 holds the 20-byte name `BASLUS-00662000`, allocation
state `0x51`, and file size `0x2000` — the assembled save block, written
through `open`/`write`/`close`.

The full retail sequence was observed by temporary traces (removed before
commit): `state 4 delete → state 6 create → write done → state 3 open →
read 8 → close 10`.

## Honest remaining frontier

The route still does **not** pass story `0x48`: after a successful save the
game's slot list keeps drawing the slot as "Unused File", so the menu never
leaves.  The cause is a pre-existing port issue independent of the write: the
state-2 enumeration writes the slot entry at

```text
record + dirent[0x13] * 0x44 - 0x1128        (retail 0x8004135C..0x80041360)
```

while the menu reads it through `func_800424B4` at

```text
0x800A0EF0 + card*0x418 + item*0x44          (retail 0x80042518..0x80042528)
```

For the two to coincide the host dirent would have to carry
`dirent[0x13] == 0x41 + item` (for card 0), but the host
`pe_card_fill_dirent` currently writes the 20-byte name over `+0x00..+0x13`
(psx-spx firstfile2 puts the filename there).  Reconciling the host dirent
layout with the game's `dirent[0x13]` convention is the next step; once the
slot list shows the file the menu can be exited and the route can progress.
The recorded `--route-pad` sequence also goes idle over the save-menu segment
(only the Cross auto-pulse fires), so the exact exit inputs may need to be
added to the route data.

The `[SAVEWRITE] func_8004FE58: missing slot entry` guard added by the prior
agent is still hit once for the same reason and is left in place.

## Matching leaves

None of the translated functions is registered as a matching `src/` leaf; the
matching build is unaffected (699 leaves, exact SHA-1).
