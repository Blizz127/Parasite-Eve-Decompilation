# PE-POST-MENU-CARD-BOUNDARY — the ~frame-44000 card boundary is a full-card write abort, now ported

Branch `agent/post-menu-card`, from `e4931e6b`.

## Symptom

The recorded Day-1 autopilot hard-stopped just after the save menu closed:

```
[ROUTE] frame=44000 token=A8002048 story=00000048 victories=0
[STUB:BOOTSTRAP_RET] card operation unresolved call (first invocation)
[HOST] stop_reason=unresolved-boundary
```

`--boundary-report` named the caller and target exactly:

```
caller=func_80040F80 target=0x80072774 a0=0x00000000 ... card0_state=9 A1838=1
```

## Investigation (temporary probes, since removed)

Temporary `fprintf` probes in `operation_boundary()`/`card_live_write()` and a
state-transition probe on `card_operation()` produced the whole picture:

```
[PM] card0 state=1 ...            <- func_80042020 save entry (len=0x2000, retries=0x1E)
[PM] card0 state=4 ...            <- open(name,0x10200) + close
[PM] card0 state=6 ...            <- open(name,2) -> state 9
[PMW] handle=0 buf=0x8009EED0 a2=1024 len=8192   r=1024     <- 7 full 1024-byte chunks
...
[PMW] handle=0 buf=0x800A0AD0 a2=1024 len=1024   r=896      <- block boundary (8064)
[PMW] handle=0 buf=0x800A0E50 a2=128  len=128    r=0        <- 30 retries, card full
[POSTMENU] boundary caller=func_80040F80 target=0x80072774 card0_state=9
```

and the resulting card image described the cause:

```
0 used=0x51 size 8192 first 1  BASLUS-006620000000A
1 used=0x51 size 8192 first 3  BASLUS-006620000000B
...
6 used=0x51 size 8192 first 13 BASLUS-006620000000F
7 used=0x51 size 0x1F80..     first 15 BASLUS-006620000000I
used blocks: [1..15]
```

So the previous run had already saved seven 2-block files (slots A..G); the
autopilot then confirmed an eighth save into the next empty slot. A 128 KiB
card has 15 data blocks, so the eighth 2-block save genuinely runs out of
room: the last 1024-byte `func_80072764` chunk stops at the block boundary
(8064 bytes) and the remaining 128 bytes cannot be allocated. `card_io_fail()`
exhausts the 0x1E retries and calls `func_80040F80(record)` — the original
abort/cleanup path — which the port still had as a boundary.

This is retail-correct behaviour, not a port bug: the game saves into a fresh
slot on every confirm, and eight Parasite Eve saves do not fit on one card.

## Fix

1. `func_80040F80` (0x80040F80..0x80041108) is now translated from
   `asm/disc1/307CC.s` in full instead of stopping at its first two callees:
   - close the record handle (`func_80072774`) and invalidate it;
   - for an aborted state-9 write, rebuild the name (save formatter, then
     `"bu%ld0:%s"`), retry `func_80072734(name,1)` up to ten times and, if it
     opens, close and erase (`func_800727A4`) the partial file;
   - run the original teardown `func_8004D5CC(index)` and zero the record
     (`record[1]`, `record[4]`, `D_800A1854`, `D_800A1838`, `func_80062CE4`).
   With no card mounted the two libcard steps keep the recorded libcard
   boundary the card oracles pin (the same absent-card replay convention the
   dispatcher already uses).
2. The formatter `%s` path no longer stops on BIOS `A(1Bh)`/`A(2Eh)`. The two
   retail thunks are ported natively in
   `pc_port/game/boot/func_80072314_port.c`:
   - `func_80072314` = `A(1Bh) strlen(src)`
   - `func_80072324` = `A(2Eh) memchr(src,scanbyte,len)`
   both cited to psx-spx ("BIOS String Functions", "BIOS Memory Fill/Copy/
   Compare"), implemented as SDK/BIOS host leaves like the rest of
   `pc_port/platform`. `PE_FormatterFrame`'s `%s` now follows the original
   branch structure (`#` length byte / `strlen` / `memchr` with precision).

## Verification

| check | result |
|---|---|
| `pe-native-tests` | 1392 run, 1392 passed |
| `ctest` (all suites) | 11/11 |
| live route, 80000 frames, fresh card | `stop_reason=frame-limit`, **0 unresolved boundaries, 0 bootstrap stubs** |
| formatter oracle | `PASS 880 original formatter frame executions, with the A(1Bh)/A(2Eh) string BIOS modelled` |
| card operation frame oracle | `PASS1280` |
| card operation oracle | `PASS8192` |
| card cleanup oracle | `PASS192` |

Route log after the fix (`--max-frames 80000`):

```
[ROUTE] frame=79500 token=A8002048 story=00000048 victories=0
[ROUTE] frame=80000 token=A8002048 story=00000048 victories=0
[FB] vsyncs=81215 drawsyncs=80078 presents=80000
[HOST] stop_reason=frame-limit
=== Stub Summary ===
implemented: 0  host-adapted: 4  bootstrap-return: 0  unsupported: 0
```

`story=0x48` is unchanged from 44000 onward because the recorded Day-1 pad
sequence ends there; the hard stop is gone and the frame budget is now the
only limiter.

A headless `--screenshot` at frame 46200 (just past the abort) renders the
live `m0020i` save menu over the field scene — "Select File to Save",
`Slot 1`, and `Used File` rows, i.e. the seven successful saves are now visible
in the slot list — rather than a black or crashed frame:
`build/artifacts/post_menu_card_abort_frame46200_2026-09-20.png` (56393/76800
non-black pixels).

Card image after the fix is unchanged in kind (7 good saves + the partial
eighth) because the original abort path's erase rebuilds the name with a
*double* device prefix (`"bu00:" + D_8009EE70`, which already contains
`"bu00:"`), so the BIOS open fails there too and the erase is skipped: this is
the original execution's behaviour, reproduced by the regenerated
`retail_card_operation_frame_cases.h` hashes (which include `0x8009EE70` and
the frame's name buffer).

## Files

- `pc_port/game/boot/func_80041108_port.c` — live `func_80040F80`
- `pc_port/game/boot/func_80072314_port.c` — new, BIOS `strlen`/`memchr` thunks
- `pc_port/game/boot/func_80071A84_port.c` — formatter `%s`
- `pc_port/include/pe_port_compat.h`, `pc_port/CMakeLists.txt` — wiring
- `pc_port/tools/pe_battle_hud_oracle.py` — BIOS `A(1Bh)`/`A(2Eh)` model
- `pc_port/tools/pe_formatter_frame_oracle.py`,
  `pc_port/tools/pe_card_operation_frame_oracle.py` — those calls are modelled,
  no longer stops
- `pc_port/tests/retail_formatter_frame_cases.h`,
  `pc_port/tests/retail_card_operation_frame_cases.h` — regenerated
  (64 formatter `%s` cases and 80 card cases now run past the BIOS calls)
