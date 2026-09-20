# PE-SAVE-MENU-WINDOW-STATE — why the Day-1 `M0020I` save menu never leaves the screen

Branch `agent/menu-window` (base `d2c94e09`, main HEAD at start).
Worktree `/tmp/pe-agent-winstate`. Ship SHA-1 authority unchanged
(`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`); no `src/` or `configs/`
file is touched by this change.

## 1. Observable

`--headless --route-pad --max-frames 42000` on the real Disc 1 with a
present 128 KiB card model:

```
[ROUTE] frame=19500 ... story=00000048
[ROUTE] frame=42000 ... story=00000048
[HOST] stop_reason=frame-limit
```

The route reaches the Day-1 field `M0020I` (token `A8002048`) at ~frame
20500, drives the whole memory-card interaction, and then parks: story
stays `0x48` for ~21500 frames.

## 2. The window state machine *does* run to completion

Instrumented run (`/tmp/ws_probe4.log`, `--max-frames 39500`), card state
transitions logged from `func_80041108`:

```
[CARD] idx=0 st=1  r0=01 r7=01 r20=8192   <- func_8004298C arms a save op
[CARD] idx=0 st=4  ...                    <- open(name,0x10200)+close (erase)
[CARD] idx=0 st=6  ...                    <- open(name,2) create
[CARD] idx=0 st=9  ...                    <- write 0x2000 in 0x400 chunks
[DELAY] cb=00000000                       <- func_8004D9D8 + func_8004CC50(0x53)
[CARD] idx=0 st=3 -> 8 -> 10 -> 3 -> 12   <- header re-read of the new file
```

`build/pe_card1.mcr` is written (131072 B, `MC` header, `BASLUS-00662…`,
state `0x51`, size 8192). The save- write chain is therefore complete and
`func_80041108` ends in its terminal state 12 with `D_8009D030 == 2`
(menu open).

## 3. Why the slot list then stops accepting the autopilot's Cross

`func_8006346C(list)` (retail `0x8006346C`) returns the *selected* row only
when the row's enabled bit is set in the `node+0x74` bitmask that
`func_800634D4` rebuilds on every draw from the `node+0x8C` predicate. The
probe shows the mask losing bit 0 exactly once:

```
[CD4] node=800A2520 idx=0 row=0 pred=8004FE58 mask=00000007
[SAVEWRITE] func_8004FE58: missing slot entry (card=0 item=0); treating row as disabled
[CD4] node=800A2520 idx=1 row=1 pred=8004FE58 mask=00000006
[G346C] node=800A2520 idx=0 mask=00000006 -> -1
```

From then on every Cross press takes `func_8004D6D4`'s `goto unable` arm
(`func_800526C4`), so the slot list can never be confirmed again.

### Root cause (retail-exact)

* `func_80042020` (save-write entry) clears the slot entry's `+1` flag:
  retail `0x80042108  sb $zero, 0x1($s3)` where
  `s3 = record + slot*0x44 + 0x1C` (retail `0x80042068..0x8004207C`).
  The port has the same store (`PE_StoreU8(s3 + 1u, 0u)` in
  `pc_port/game/boot/func_80042020_port.c`).
* `func_800424B4` (retail `0x800424B4`) returns `0` when that flag is zero
  (`0x80042508 lbu $v0, 0x800A0EF1($at)`), so it returns the null pointer
  for the slot that was just written.
* `func_8004FE58` (retail `0x8004FE58`, `0x8004FE84`) then does
  `lbu $v0, 0x0($v0)` **without a null check** — it reads the byte at
  physical address `0`.

So retail's post-save row state is "read the byte at address 0". The port's
`func_8004FE58` guard (added by `agent/card-write`) returns "row disabled"
instead, because `PE_LoadU8(0)` aborts in the host RAM model
(`PE_RAM_BASE == 0x80000000`, 2 MiB, KSEG0 mirror only).

**Open fidelity item.** The faithful emulation is to read the RAM byte the
PS1 mirrors at `0x80000000` when `func_800424B4` returns 0, i.e.
`entry ? entry : 0x80000000u` before the `PE_LoadU8(entry)`. This report
does not apply it because (a) the port's low 64 KiB is only the zeroed
calloc image (the PS1 kernel's exception vector is not modelled), so the
emulated byte is 0 → `(0 ^ 3) != 0` → *enabled*, which would make the
just-written row selectable again and route a confirm into
`func_8004D6D4` with a null `entry` (retail then dereferences address 0
for `D_8009CF4C`), and (b) that second null-dereference site would need the
same treatment. Both are documented here rather than invented.

## 4. The menu *can* be closed — and what closes it

`func_8004D6D4` handles the cancel event *before* the confirm event:

```c
if ((event & 0x40u) != 0u) {           /* Circle */
    func_80062F3C(0x3Fu); func_80062F1C(window);
    func_80042A10(); func_80052634(); return 1;
}
```

and `func_8004D2DC` (the card-select window `0x24`) runs

```c
func_8005C1EC(0);        /* D_8009D030 = 0 */
func_800512AC(9, 0);     /* D_8009D010 = 0xFFFFFFFF (menu result "cancel") */
```

so `PE_FieldMenuFrame`'s `if ((uint16_t)result)` teardown fires.

Demonstrated live by injecting Circle presses through
`PE_ROUTE_PAD_SEQUENCE` (`/tmp/ws_exit7.log`, pattern file
`/tmp/ws_seq6.txt` — the recorded route with two Circle press edges every
16 frames from 39110):

```
[WIN0x25] ev=40 sel=-1 base=00000001     <- slot list cancel
[CARD] idx=0 st=0 ...                    <- func_80042A10 reset
[WIN0x24] ev=40 sel=0                    <- card-select cancel
[MENUCMD] cmd=9 src=00000000 d010=FFFFFFFF d030=0
[DBGMENU] n=40001 res=FFFFFFFF ... d030=0
```

The menu state is clean: timer 0, result `0xFFFFFFFF`.

## 5. The actual remaining blocker

Immediately after that close the menu re-opens on its own:

```
[DBGMENU] n=40002 res=00000000 ... d030=0
[DBGMENU] n=40224 res=00000000 ... d030=2     <- reopened, ~224 frames later
[MENUCMD] cmd=9 ... d010=FFFFFFFF d030=0      <- closed again
```

The only two `func_8005C1EC(1)` callers are `func_801909B4` (boot) and
`func_8004D18C` (the script opcode `E7`, dispatched by
`func_80017018_port.c`). Retail `func_80015AF0` (`0x80015AF0`) is:

```
lw   v0, 0(D_800B0CD8); andi v0,v0,0x1000; bnez -> return 1
lhu  v1, 8(task);       andi v0,v1,0x20;  bnez -> .L80015B54
   ... task[8] |= 0x20; ... ; return 0
.L80015B54:
   func_80067CBC(); func_8004D18C();
   D_800B0CD8 |= 0x9000; D_8009D1A0 |= 4; task[8] &= ~0x20; return 1
```

`func_8005C1EC(0)` clears `D_800B0CD8` bits `0xC000` and the field-menu
teardown clears `0x9000`, so the `0x1000` guard stops rejecting the next
`E7` entry and the *script* re-issues the opcode.

So the stall is not a window-state defect: the Day-1 `M0020I` script
re-enters `E7` until the card op reports a non-cancel result. A cancel
yields `D_8009D010 == 0xFFFFFFFF`; the "success" result `2` is published
only by `func_800512AC(12,0)`, which the port reaches through
`func_80042228`, and `func_80042228` is installed by `func_80042264` —
the **load-path** CRC-success tail (`0x80042430`). The save path
(`state 9 -> 3 -> 8 -> 10 -> 3 -> 12`) never runs it. Resolving that is a
script/semantics task outside this window-state scope; this report leaves
the evidence rather than faking a result.

## 6. Changes shipped here (port fidelity only)

1. `pc_port/game/boot/func_80041108_port.c` — `case 12` was
   `case 12:return;`. Retail `.L80041FE8` runs the cleanup when the record
   is not flagged live:

   ```c
   lbu v1, 0(s0); v0 = 1; beq v1, v0 -> epilogue
   .L80041FF8: jal func_80040F80
   ```

   Now `if (PE_LoadU8(record) != 1u) func_80040F80(record);`.

2. `func_8004D5CC` (`0x8004D5CC`), `func_80042B6C` (`0x8005C4E4`), and
   `func_8004D030` (`0x8004D030`) all publish `D_8009CFFC` through an
   unqualified indirect call in retail (`jalr $v0`). The port's
   hand-written switches omitted two guest targets it can reach:
   `func_800428D4` (`0x800428D4`, arms record state 13) and
   `func_80042228` (`0x80042228`, the CRC-ok close publisher). Both are
   now dispatched instead of stopping on a spurious
   "unported callback" boundary. Every still-unknown target keeps its
   loud boundary.

Deliberately **not** changed: the `func_80040F80` libcard-close boundary.
Retail `0x80040FC0..0x80040FD4` closes the open handle with
`func_80072774` and invalidates `record+0xC`, but the retail oracle
`pc_port/tests/retail_card_cleanup_cases.h` (`test_DAY1_card_cleanup`)
pins that first callee as the recorded boundary, so it stays
(`pe-native-tests` 1392/1392 after reverting that experiment).

## 7. Verification

| Gate | Result |
| --- | --- |
| `pe-native-tests` | 1392 run / 1392 passed / 0 failed (`/tmp/ws_final` shell log) |
| `ctest --test-dir pc_port/build` | 11/11 passed, 141.09 s |
| `gen_decomp_ports.py --check --allow-orphans` | `check: OK` (0 stale, 0 mismatched) |
| `git status` | only the three `pc_port/game/boot/*.c` files above |
| matching build | untouched — no `src/`, `configs/`, `asm/`, or `scripts/` change |

Live route after the change (default recorded pad, present card):
`story 0x48`, `stop_reason=frame-limit`, 4 host-adapted stubs, 0 bootstrap
stubs — identical to the pre-change baseline (no regression, no new
boundary).

## 8. Raw logs kept

`/tmp/ws_probe4.log` (state machine + draw mask), `/tmp/ws_exit5.log`
(first live `[WIN0x25] ev=40`), `/tmp/ws_exit7.log` (complete close:
`cmd=9`, `d030=0`, then the script re-entry), `/tmp/ws_ctest.log`.
