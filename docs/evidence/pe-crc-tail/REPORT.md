# CRC tail / menu close — investigation result

Branch `agent/crc-tail` (worktree `/tmp/pe-agent-crctail`), base
`e002ef5a` (706 matching leaves, `agent/menu-result` merged).  Authority:
retail Disc1 EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` and the
split sources under `asm/disc1/`.

Task premise under test: port `func_80042264` (the read/CRC-verify tail), fix
the `func_80040F80` flag address (`0x800A1DB4` vs `0x800A185C`), address the
Cross-release activate loop, and make the Day-1 `--route-pad` route pass story
`0x48`.

## 1. The `0x800A1DB4` lead is FALSE (verified)

Retail `func_80040F80` (`asm/disc1/307CC.s:1128`, span `0x80040F80..0x80041108`)
checks **`D_800A185C`**, not `D_800A1DB4`:

```text
31780: lui  v0,%hi(D_800A185C)
31784: lw   v0,%lo(D_800A185C)(v0)
317A0: beqz v0,.L80040FB8          ; if (D_800A185C) -> close path
317A8: jal  func_80042228          ; THE close publisher
```

A whole-tree scan of `asm/disc1/` finds **no `D_800A1DB4` symbol and no write to
offset `0x1DB4`** (the only `1DB4` strings are unrelated file offsets in other
units).  `[0x800A185C]` is written at `0x80042410` (inside `func_80042264`) and
cleared at `0x800425C0` (inside `func_800425DC`).

**Conclusion:** the port's existing `if (PE_LoadU32(0x800A185C)) { func_80042228(); ... }`
in `func_80040F80` is correct; the reported address discrepancy was a
misdiagnosis and no change is warranted.

## 2. `func_80042264` is the LOAD-path tail, not the save path (verified)

The state dispatcher of `func_80041108` indexes `jtbl_80010F6C` by
`record+1` (`asm/disc1/307CC.s:1255`).  Table `asm/disc1/data/800.rodata.s:1234`:

```text
state 7  -> 0x80041B18
state 8  -> 0x80041A58
state 9  -> 0x80041C0C   (save write)
state 10 -> 0x80041D04
```

* `state 8` (`0x80041A58`) caps the read at `0x80` and on completion sets
  `record+1 = 10` (`sb v0,0x1(s0)` at `0x80041AC8`).
* `state 7` (`0x80041B18`) caps the read at `0x400` and on completion closes
  the handle, sets `record+1 = 12`, clears `D_800A1854`/`D_800A1838`, and calls
  `func_80042264` at `0x80041BB0`.

Retail **state 9** (save write) tail (`0x80041C88`) sets `record+1 = 3` and
calls `func_8004CC50(0x53, 0)`.  State 3 then opens the file for read (header,
`state 8`), and the flow is state 9 -> state 3 -> state 8 -> state 10 -> state 3
-> 12.  It **never enters state 7**, so the save path never calls
`func_80042264` and never sets `[0x800A185C]` itself.

`func_80042264` (`asm/disc1/32A64.s`, `0x80042264..0x80042464`) does:
1. copy `0x12E0` bytes `D_8009EFD0 -> D_800C0DE0`; `D_800A0ED0 += 0x12E4`;
2. `jal func_8003FBD8` (record field loader, `0x3D4`, pure data movement, no
   `jal` of its own);
3. CRC-16/CCITT (`0x1021`) over `0x2000` bytes at `D_8009EED0` and compare with
   the stored first word;
4. on match: `func_8005C374`, `[0x800A185C] = 1`, `func_8004D9D8`,
   `func_8004CC50(0x54,0)`, `func_8004D024(func_80042228)`;
   on mismatch: `func_8004D9D8`, `func_8004CE28(0x55,0x56)`.

Its two large dependencies (`func_8003FBD8` `0x3D4`, `func_8005C374` `0x114`)
are unported, so a faithful port of the CRC tail requires porting them first.
Because it is off the save route, that work would not change the route result.

## 3. The pad -> menu mapping (measured)

`func_8005E038` (`pc_port/game/boot/func_8005E12C_port.c`) maps the abstract pad
`D_8009D26C` to menu buttons.  `func_8003EB04` builds `D_8009D26C` from the BIOS
pad.  Measured on the live route (temporary `[PAD]` trace, reverted):

| BIOS pad (active-low) | button  | abstract `D_8009D26C` | menu event |
|---|---|---|---|
| `FFFF` | none | `00000000` | — |
| `FFEF` | Up | `00000008` | `0x1000` |
| `FFDF` | Right | `00000010` | `0x2000` |
| `FFBF` | Down | `00000020` | `0x4000` |
| `FF7F` | Left | `00000040` | `0x8000` |
| `EFFF` | Triangle | `10000080` | `0x10|0x80` |
| `BFFF` | Cross | `20000300` | `0x20` (confirm) |
| `DFFF` | Circle | `40000401` | **`0x40` (cancel)** |

`func_8005E30C` promotes a type-4 release of menu bit `0x20` to the synthetic
`0x10000` activate event (`func_80063E0C_port.c:226`), which is faithful to
retail `0x8005E480`.

## 4. The save-menu exit is the real `0x48` blocker (measured)

The `--route-pad` auto-pulse presses Cross every 8 frames, producing menu
`0x20`/`0x10000` confirms.  Experimental runs (all reverted) showed:

* Injecting BIOS **Circle** (`0xDFFF`) at frames 38680..41080 **does** deliver
  menu `0x40` cancel events; handlers `func_800452C0` and `func_80044B0C`
  consume them (`ret=1`), yet the game still never leaves story `0x48`.
* Suppressing the auto-pulse over `[38300,42001)` (temporary
  `g_pulse_end`/`g_pulse_resume` edit) also does not advance the story.
* A Triangle press (`0xEFFF`) produces only menu `0x10|0x80`, not a cancel.

So the blocker is the **save-menu window state machine after a successful save**,
not input delivery and not the CRC tail.  The recorded route also goes idle over
the save-menu segment, so its exit inputs are not part of the recording.

## Verified state

```text
baseline (base e002ef5a): --route-pad --max-frames 42000
  [HOST] stop_reason=frame-limit
  [ROUTE] frame=42000 token=A8002048 story=00000048
```

No `src/`/`configs/` change was made; the matching build is untouched.

## Non-claims

No route advance past story `0x48`; `func_80042264` is **not** ported; no
matching-leaf change; no claim that the `--route-pad` sequence as recorded can
leave the save menu.

## Recommendation

The next concrete step is an evidence-led trace of the **save-menu window
state**: after the save completes, which window owns focus, what the cancel
handlers `func_800452C0`/`func_80044B0C` actually do, and what state the menu
result `[0x8009D010]` must reach for `func_8005C498` to return a close value.
Porting `func_80042264` is still worthwhile for LOAD-path fidelity, but it needs
`func_8003FBD8`/`func_8005C374` first and will not by itself pass `0x48`.
