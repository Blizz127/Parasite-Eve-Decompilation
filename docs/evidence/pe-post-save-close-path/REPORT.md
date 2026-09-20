# Post-save close path — trace result

Branch `agent/menu-result` (worktree `/tmp/pe-agent-menuresult`, base `34a4acbc`,
706 matching leaves). Scope: find the missing step that publishes the menu
result / closes the save menu after a successful save.

## What was verified

### 1. The `func_8004DCA4` hint is real code but OFF the executed path

Retail `func_8004DCA4` (`3E4A4.s`, 0xC0) disassembly (rebuilt byte-exact ELF
`build/disc1.elf`):

```
8004dcdc: beqz  s0,8004dd18        ; mode == 0 -> func_8004DD64(-1)
8004dce4: jal   80054294           ; func_80054294() = D_8009D068
8004dcec: beqz  v0,8004dd08        ; return 0 -> close
8004dcf4: li    a1,-2
8004dcf8: jal   80048918           ; nonzero -> func_80048918(0,-2,-1)
8004dd08: jal   8005c488           ; THE ONLY direct func_8005C488 call in the EXE
```

An exhaustive `objdump` scan of the EXE finds exactly one direct
`jal 8005c488` (at `0x8004DD08`); every other use is an indirect callback.
So the branch is faithfully retail-shaped, and it was implemented here.

**But it is never executed by the route.** With a temporary entry probe,
`func_8004DCA4` was called **0 times** across the full 42000-frame
`--route-pad` Day-1 run. The hint (from the idle agent's saved patch
`/tmp/pe_menu_close_hint.patch`) is therefore a correct transcription of a
branch the save menu does not use. It is kept because it is retail-proven and
inert on this route.

### 2. The save DOES complete and DOES create its notice

Live trace (instrumented, then reverted) of the real Disc-1 present-card run:

```
[MR] f4CC50 first=53 second=00        ; save-complete notice (retail 0x80041C0C)
[MR] f4D024 cb=00000000               ; func_8004D024(0) -> D_8009CFFC cleared
[MR] f4D030 event=00010000 cffc=00000000 d034=00000000 d010=00000000
```

`build/pe_card1.mcr` is written (`BASLUS-00662000`). State 9
(`0x80041C0C..0x80041CA8`) matches the port exactly: write, close,
`state=3`, `record+7=0`, `[0x800A1854]=0`, `func_8004D9D8()`,
`func_8004CC50(0x53,0)`.

### 3. The close callback is only armed on the LOAD/CRC path

`func_8004D024(0x80042228)` is called from exactly one place, inside
`func_80042264` (retail `0x80042428`):

```
80042400: jal 8005c374    ; CRC check
80042408: li  v0,1 ; sw v0,6236(at)   ; [0x800A185C] = 1
80042414: jal 8004d9d8
8004241c: li  a0,84       ; notice 0x54
80042420: jal 8004cc50
80042428: la  a0,80042228
80042430: jal 8004d024    ; arm the close callback
```

`func_80042228()` (retail `0x800422F0`) frees menu nodes 38/37/36 and calls
`func_800512AC(12)` -> `[0x8009D010]=2`, which is what would close the menu.
**This is the load/CRC-verify tail.** It is reached from state 7's read
completion; the SAVE path (state 9 -> state 3 -> open(1) -> state 8 read ->
state 10) never enters state 7, so it never arms the callback.

### 4. The notice is created but never receives a dismiss event

`func_8004D030` is called exactly **once** (with the synthetic open event
`0x10000`); `func_8004CC50(0x53,0)` armed no callback, so the dismiss does
nothing. Meanwhile the focused slot-list handler is re-entered on every Cross
release:

```
[MR] mcb fn=80063E0C a0=800A2520 a1=00000020
[MR] mcb fn=80063E0C a0=800A2520 a1=00010000
[MR] mcb fn=8004D6D4 a0=800A2490 a1=00010000     <- repeats to frame 42000
```

`func_8005E30C` maps a type-4 release of the 0x20 (Cross) bit to the synthetic
`0x10000` "activate" event, which `func_8004D6D4` treats as a confirm
(`func_8004D4C4_port.c:188`). So the menu re-enters the confirm path forever
and never closes.

### 5. Address discrepancy worth fixing (not fixed here)

The port's `func_80040F80` checks `[0x800A185C]`:

```c
if(PE_LoadU32(0x800A185Cu)) {func_80042228();return;}
```

Retail `func_80040F80` (`0x80041044`) checks `[0x800A1DB4]`
(`lui v0,0x800a; lw v0,7604(v0)`), which `func_80042264` sets at `0x800424DC`
and `func_800425DC` clears at `0x8004268C`. `0x800A185C` is a *different* word
(the CRC-success latch). This is a lead, but because `func_80042264` is still
unported the flag is never set on the save path, so it is reported rather than
guessed at.

## Precise missing step

Port **`func_80042264`** (retail `0x80042264`, the read/CRC-verify tail that
sets `[0x800A185C]` and `[0x800A1DB4]`, shows notice `0x54`, and arms
`func_8004D024(0x80042228)`), and reconcile the `func_80040F80` flag
(`0x800A1DB4` vs `0x800A185C`). That is what publishes the menu result
`[0x8009D010]=2` and lets the menu leave story `0x48`. The save-write chain
itself is complete and the `.mcr` is correct.

## Change in this branch

Only the retail-proven `func_8004DCA4` mode!=0 branch (plus its
`func_80054294` prototype). It is inert on the current route and does not
change the route result (42000 frames, story `0x48`), so it is a correctness
fix, not the unlock.

## Verification

```
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build
./pc_port/build/pe-native-tests      -> 1392 run / 1392 passed / 0 failed / 0 skipped
ctest                                -> 11/11 passed
./pc_port/build/parasite-eve-port --disc-image "$(cat local/pe_disc1.path)" \
    --headless --route-pad --max-frames 42000 --boundary-report
    -> stop_reason=frame-limit, story 0x48 (unchanged; expected)
```

## Non-claims

No menu exit, no story advance past `0x48`, no matching-leaf change. The
`func_8004DCA4` fix is retail-proven but off-path.
