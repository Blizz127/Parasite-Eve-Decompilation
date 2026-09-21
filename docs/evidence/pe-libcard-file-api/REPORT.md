# libcard file API + card-operation state-2 enumeration

Branch `agent/libcard-file-api`, base `b28eb1a1`. Worktree
`/tmp/pe-agent-libcard`.

## What the wall was

The present-card model reached the game's **Select Slot** save menu and stopped
at the named `card operation unresolved call` — `func_80041108` state 2
calling `func_800727B4` (`firstfile`). Two things were missing: the libcard
file API and the state-2 post-call enumeration.

## Implemented

### libcard file API (`pc_port/platform/pe_libcard.c`)

Host model over the present 128 KiB image, following the psx-spx Memory Card
Data Format (block 0: header frame 0 + 15 directory frames 1..15; blocks 1..15:
a 128-byte block header then 63 data frames = 8064 bytes each; entry = 32-bit
alloc state +0x00, 32-bit size +0x04, 16-bit next block +0x08, 20-byte name
+0x0A, XOR checksum +0x7F):

| BIOS | port function | behavior |
| --- | --- | --- |
| B0 32h open | `func_80072734` | find/create a directory entry; returns an fd or -1 |
| B0 33h lseek | `func_80072744` | set/seek the file position |
| B0 34h read | `func_80072754` | copy from the block chain |
| B0 35h write | `func_80072764` | copy into the chain, grow it, update size |
| B0 36h close | `func_80072774` | finalise the entry checksum and persist the image |
| B0 41h format | `func_80072784` | reformat the image |
| B0 42h firstfile | `func_800727B4` | enumerate the first named directory entry |
| B0 43h nextfile | `func_80072794` | advance the enumeration |
| A0 18h memcmp | `func_80071A04` | 12-byte name compare the directory scan uses |

Nothing is faked: an absent or empty card returns the documented `-1`/`0`.

### `func_80041108` state 2 (`pc_port/game/boot/func_80041108_port.c`)

Translated from `asm/disc1/307CC.s` 0x800412C0..0x800415AC:

- writes `index+0x30` to `[D_80092230]+2`, clears record bytes 2/3/6/4/7/0A,
  sets the 15 entry-type bytes to 2;
- calls the real `firstfile`; on failure decrements the retry halfword and, if
  its signed value is still positive, returns; otherwise scans;
- for each returned entry, a 12-byte name match with `[D_80092224]+6` sets that
  entry type 1 / read-marker 1 / variant byte and `record+4`, then adds
  `size>>13` blocks;
- accounts the entries (type 2 -> read-marker 1, others reduce the block count;
  type 2 -> 3 until the block count is exhausted), sets `record+2=15`;
- if `D_800A186C==0`, sets state 15 and clears busy; otherwise searches for a
  type-1 entry and continues to state 3/timer 10 when one exists (or blocks<15
  and `func_8004D27C()!=0`), else takes the unable path.

The original dirent output is the caller's `frame+0x20`; the context-free live
entry has no frame, so it reuses the card's own transient filename buffer
`0x8009EE70` (documented in the code, not a fabricated stack address).

### Explicit new frontier

Both state-2 continuations call the slot-list UI (`func_8004D4C4` on the
continue path, `func_8004D298` on the unable path). Those read the PS1
**low-memory kernel/menu substrate** (address 0 / 0x150) that the port does not
model — the fixture faulted with `PE_LoadU32(0)`. They stay explicit named
boundaries rather than faulting or being faked.

## Oracle/test changes

- `pe_card_operation_oracle.py` / `pe_card_operation_frame_oracle.py`: STOPS
  drop `0x800727B4` and add `0x8004D4C4` / `0x8004D298`; headers regenerated.
- `pe_battle_hud_oracle.py`: new optional `hook_at` PC-hook map (default off)
  that simulates a leaf call with no guest stack frame; the card oracles hook
  `func_800727B4` to return 0 (the RAM file table at 0x150/0x154 is empty in
  the fixtures), matching the host implementation which allocates no guest
  stack frame.
- `test_card_status.h`: new `DAY1_card_file_api` creates a file, writes 200
  bytes, closes, enumerates it with firstfile/nextfile (name + size), reopens
  read-only and round-trips the bytes.
- `test_decomp_ptr_params.h`: `DECOMPPTR_boundary_guest_addresses` now checks
  the real memcmp (equal and differing 12-byte names) because `func_80071A04`
  is implemented; the generated `func_800816F4` TU direct-calls it.

## Proof

```
cmake --build pc_port/build -j
./pc_port/build/pe-native-tests
  Results: 1378 run, 1378 passed, 0 failed, 0 skipped
ctest                -> 11/11 passed
```

Live route (`--headless --route-pad --max-frames 42000 --boundary-report`):

Before (base `b28eb1a1`):

```
[ROUTE] frame=38500 token=A8002048 story=00000048
[STUB:BOOTSTRAP_RET] card operation unresolved call   (func_800727B4 firstfile)
[HOST] stop_reason=unresolved-boundary
```

After:

```
[ROUTE] frame=38500 token=A8002048 story=00000048
[CARDDBG] func_80041108 target=8004D4C4                (transient diagnostic)
[STUB:BOOTSTRAP_RET] card operation unresolved call
[HOST] stop_reason=unresolved-boundary
```

The route now runs the directory enumeration and stops at the **slot-list UI
constructor `func_8004D4C4`** instead of `firstfile`. Story is still `0x48`.

## What is real vs still named

- **Real:** the whole libcard file API over the present image; the state-2
  directory enumeration, name match, block accounting and dispatch; the A0(18h)
  memcmp.
- **Still named:** the slot-list UI constructor/fallback (`func_8004D4C4` /
  `func_8004D298`) and the menu renderer (`func_800434C0`, `func_8004D6D4`,
  ...), which need the PS1 low-memory kernel/menu substrate. Entry transfers
  in states 3..11 and the transfer/retry paths beyond their first callee remain
  unported, as before.

## Blockers / next

The next unlock is modelling (or supplying) the PS1 low-memory kernel/menu
substrate so `func_8004D4C4`/`func_8004CE28` can run, then the menu draw
callbacks. No save/load completed, no story progress past `0x48`, and no `.mcr`
is committed (`*.mcr` stays git-ignored).
