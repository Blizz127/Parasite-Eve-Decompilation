# Memory-card present model: host-backed kernel + the save-menu frontier

Branch `agent/card-present`, base `5cf6b196`. Worktree
`/tmp/pe-agent-cardpresent`.

## What the wall was

The empty-slot kernel model (previous pass) made the card **status machine**
`func_800405A4` loop forever: with both slots empty, `_card_info` reports
`F4000001h,2000h` (eject) and `_card_write` reports `F0000011h,2000h` (I/O
err), so the machine cycles `state 0 -> 1 -> 2 -> 0` and never accepts a
record.  Instrumented every 500 frames from 38500 to 42000:

```
[CARD] s0=01 s1=00 f20=0 f24=0 f28=1 f2C=0 ... 364=0 370=FFFFFFFF 40=0
[ROUTE] frame=42000 token=A8002048 story=00000048   (stop_reason=frame-limit)
```

The record status byte `record+8` sat at 1 and `A1828` (higher-level eject)
was the only flag set, for 3500 frames.

## What was implemented

`pc_port/platform/pe_libcard.c` now models a **present** card backed by a raw
128 KiB host image, and falls back to the documented empty-slot timeout only
when `PE_CARD=empty`.

Grounding — psx-spx (`BIOS Event Summary`, `Memory Card Data Format`,
`B(5Ch) _card_status`):

| operation | veneer | success event | error event |
| --- | --- | --- | --- |
| `_card_info` (A0 ABh) | `func_8007DD44` | `F4000001h,0004h` -> A1820 | `F4000001h,2000h` -> A1828 |
| `_card_load` (A0 ACh) | `func_8007DD54` | `F4000001h,0004h` -> A1820 | `F4000001h,2000h` -> A1828 |
| `_card_write` (B0 4Eh) | `func_8007DDB4` | `F0000011h,0004h` -> A182C | `F0000011h,2000h` -> A1834 |
| `_new_card` (B0 50h) | `func_8007DDC4` | no completion event | — |

`0004h` is documented as `card done okay` / `finished okay`; `2000h` is
`card err eject` / lower-level `err`.  `func_800409B4` opens all eight with
mode 1000h, so delivery runs the matching verified callback
(`func_80042BD8`/`28`/`14`/`64`) exactly as the retail IRQ would.

Image format (psx-spx `Memory Card Data Format`): 16 blocks x 8 KiB =
1024 frames x 128 bytes; header frame 0 is `"MC"` (4Dh 43h) with byte 7Fh =
XOR of 00h..7Eh; directory frames 1..15 carry a 32-bit allocation state
(A0h = free/freshly formatted), 04h..07h filesize, 08h..09h next block,
0Ah..1Eh filename, 7Fh XOR checksum.  The image is loaded from
`PE_CARD_IMAGE`, default `build/pe_card1.mcr` (git-ignored; `*.mcr`/`*.mcd`
added to `.gitignore`), and is created and formatted on first use.
`_card_write` validates sectors per psx-spx (0..3FFh valid, 400h accepted by
the documented retail quirk, else rejected) and copies the 128-byte frame
into the image.

## Proof

Tests (`pc_port/tests/test_card_status.h`, `test_native.c`):

```
./pc_port/build/pe-native-tests
  Results: 1377 run, 1377 passed, 0 failed, 0 skipped
```

`DAY1_card_status` still pins the empty-slot oracle (native fixtures default
to empty via `ResetTestState`).  New `DAY1_card_present_kernel` verifies the
header/directory XOR checksums, the three success events (`A1820`/`A1820`/
`A182C`), that `_card_write` persists the 128-byte frame at `sector*128`, and
the sector validation (`400h` accepted, `401h` rejected).

Card oracles (original code, candidate SHA-1 `452fb033…`):

```
pe_card_status_oracle.py        PASS 4096 original card status cases
pe_card_driver_oracle.py        PASS 96 original card driver graphs
pe_card_record_oracle.py        PASS 256 original record chains
pe_card_confirmation_oracle.py  PASS 128 original confirmation graphs
```

Live route (`--headless --route-pad --max-frames 42000 --boundary-report`):

Before (empty model):

```
[ROUTE] frame=42000 token=A8002048 story=00000048 victories=0
[HOST] stop_reason=frame-limit          (infinite status loop; zero stubs)
```

After (present model):

```
[CARD] formatted a fresh 128 KiB image at 'build/pe_card1.mcr'
[ROUTE] frame=38500 token=A8002048 story=00000048
[STUB:BOOTSTRAP_RET] card operation unresolved call (first invocation)
[HOST] stop_reason=unresolved-boundary
```

At the stop the record has advanced out of the status loop:
`record+8=04` (status accepted), `record+1=02` (operation processor state 2 =
directory enumeration), `record|=1`, `A182C=1` (write done-okay).  A
screenshot at the frontier shows the game's **"Select Slot"** save menu with
its timer running — the present model gets the game to the point of
enumerating save slots.

`PE_CARD=empty` reproduces the previous loop (runs to the frame limit with
zero stub stops), so the empty path is unchanged.

## Honest remaining frontier

**The route does not yet pass story `0x48`.**  With a present card the game
opens the save menu and the operation processor `func_80041108` reaches its
first unresolved callee at state 2 — `func_800727B4` (`firstfile`, BIOS
B0-function family) — and the port deliberately stops there.  Advancing needs
two unported subsystems, neither of which this change fakes:

1. the **libcard file API** veneers (`func_80072734` open, `...54` read,
   `...64` write, `...74` close, `...84` format, `...94` nextfile, `...B4`
   firstfile), and
2. the `func_80041108` **post-call** directory/file state graph documented in
   `docs/ai_context/CARD_OPERATION_CONTRACT.md` (currently only the pre-call
   half is native).

The kernel layer is now real; the file layer is the next frontier.  Non-claims:
no save/load completed, no route progress past `0x48`, no `.mcr` committed.
