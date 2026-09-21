# Save-menu exit: card-name truncation fixed; menu-exit blocker isolated

Branch `agent/menu-exit`, worktree `/tmp/pe-agent-menuout`, base `69261444`
(706 matching leaves).  Authority: retail Disc1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Confirmed bug: the host truncated the card filename before stripping "buXX:"

The retail save filename is produced by the formatter template

```text
[0x80092224] -> "bu%d0:BASLUS-00662000000%c%c"
```

i.e. a 6-char device prefix `bu00:` plus a **20-char filename**:

```text
name[0x00..0x0B] = "BASLUS-00662"
name[0x0C..0x11] = "000000"
name[0x12]       = variant  ('0', from entry[0x29]+'0')
name[0x13]       = 'A'+slot
```

The game's state-2 directory match (`func_80041108` @ `0x800412C0`, retail)
reads the BIOS direntry at `name[0x12]` (variant, compared `^0x30`) and
`name[0x13]` and maps the entry with

```text
entry = record + dirent[0x13]*0x44 - 0x1128       (retail 0x8004135C)
```

which equals the `func_800424B4` slot-entry base `0x800A0EF0 + card*0x418 +
item*0x44` exactly when `dirent[0x13] == 0x41 + item` (both bases differ by
`0x1C`, and `0x1128 + 0x1C == 0x41*0x44`).

The host's `pe_card_name_from_guest` copied at most **20 bytes first** and only
then stripped the device prefix, so the 26-char guest name
`"bu00:BASLUS-006620000000A"` became the 20 bytes `"bu00:BASLUS-00662000"` and,
after stripping, the 15-char `"BASLUS-00662000"` — `name[0x12]`/`name[0x13]`
were zero.  The card was therefore written with a 15-char name, and the game's
directory match read slot 0 for every file.

Live probe (temporary `[DBGNAME]` trace) confirmed the formatter itself is
correct:

```text
[DBGNAME] fmt='bu00:BASLUS-006620000000A...' variant=30 slot=0 arg1=41 card=0
```

### Fix

`pe_card_name_from_guest` now reads the whole guest name into a 40-byte buffer,
finds the `:` device separator, then copies 20 bytes **after** it:

```c
raw[40] <- guest string (NUL-terminated)
src = (colon seen within 6 chars) ? colon+1 : 0
out[0..19] = raw[src..src+19] (zero-filled past the end)
```

The created card entry name is now `"BASLUS-006620000000A"`; dumping
`build/pe_card1.mcr` frame 1 shows `name[0x12]=0x30`, `name[0x13]=0x41`.

### Regression test (discriminating)

`DAY1_card_device_prefix_name` (`pc_port/tests/test_card_status.h`): formats a
present card, creates a file with the **device-prefixed** name
`"bu00:BASLUS-006620000000A"`, enumerates with `func_800727B4`, and asserts the
dirent's `+0x12 == 0x30` and `+0x13 == 0x41`.

Measured discrimination:

```text
with fix:     1392 run / 1392 passed / 0 failed
without fix:  PE_TEST_FILTER=DAY1_card_device_prefix_name ->
              Results: 1392 run, 0 passed, 1 failed, 1391 skipped
```

Full verification with the fix:

```text
./pc_port/build/pe-native-tests   -> 1392 run / 1392 passed / 0 failed / 0 skipped
ctest                             -> 11/11 passed
card oracles (status/operation/operation_frame/driver/record/confirmation)
                                  -> PASS (4096/8192/1280/96/256/128)
```

No `src/`/`configs/` change: the matching build stays EXACT SHA-1
`452fb033...` (706 leaves).

## Remaining blocker: the menu still does not leave (story stays 0x48)

With the corrected name the saved slot is enumerated as **occupied**
(`entry[0]=1`, `entry[1]=0`) and the accounting loop leaves `entry[1]=0`; the
retail free-slot lookup `func_800424B4` (requires `entry[1] != 0`) therefore
does not return the saved slot, which matches a "Select File to Save" menu that
lists free slots.  `func_8004FE58` logs `missing slot entry (card=0 item=0)`
once and disables that row.

Traced with temporary probes (`[DBGST]`, `[DBGDRV]`, `[DBGMENU]`, removed before
commit):

```text
[DBGST]  func_80041108 runs 34 times total: state 1 -> 2 -> 3 -> 12 -> 1 ->
         4 -> 6 -> 9 (write) -> 3 -> 8 -> 10 -> 3 -> 12, then stops being called.
[DBGDRV] func_800425DC is entered every frame (3400+ times) with record 0 state 1.
[DBGMENU] frame-limit state: t030=2 r010=0 a1838=0 a1864=0 a1840=FFFFFFFF
          a183C=1 ced8=0
```

So `func_800425DC` keeps running but `func_80041108` is no longer reached
(an early `PE_Port_StopEpoch` return inside `func_800425DC`), and the menu
result `[0x8009D010]` stays 0, so `func_8005C498` never returns a "close"
value.  Injecting a single Triangle / Circle / Start / Select / Square press
over frames 38800..39100 (temporary `PE_MENU_EXIT_PAD` probe, removed) does
**not** change story `0x48`, so the exit is not a simple missing button.

**Next step:** instrument the `func_800425DC` epoch check to find which call
(`func_800405A4(1)`, `func_800405A4(0)`, or `func_80041108(1)`) bumps the stop
epoch, and confirm against the retail `func_800425DC` (`asm/disc1/307CC.s`
`0x800425DC`) whether the host's `PE_Port_StopEpoch` guard is a false early-out.
