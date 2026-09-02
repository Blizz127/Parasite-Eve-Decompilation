# PE-B54K-AQ — retail boot path from the movie player to the first input wait

Status: **STATIC RETAIL MAP COMPLETE; NO NATIVE CODE CHANGED BY THIS RUNG**.

Authority: `SLUS_006.62` SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`;
`PE.IMG` SHA-1 `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`; the boot overlay
is PE.IMG sectors `[0x03D2,0x0457)` loaded at `0x8018EFF0` (B54K-O/P). All
overlay addresses below are read from that image at
`PE.IMG[0x1E9000 + (addr - 0x8018EFF0)]`. Disassembly used capstone 5.0.7
over the two images; every `jal` census scanned both the complete
executable text and the complete overlay.

## Span headers

```text
func_801909B4 overlay main        [0x801909B4,0x801918F8) 977 words
SHA-256 9072713338b26c335c1a31964282105dcd5f14951950c2d24585fa5948554d30
func_80192CE8 movie sequence      [0x80192CE8,0x80192F98) 172 words
SHA-256 ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7
func_801924F8 movie player        [0x801924F8,0x80192934) 271 words
SHA-256 ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a
func_80192934 per-frame step      [0x80192934,0x80192CE8) 237 words
SHA-256 bd9d3e37b7bb80c471b2c3592035615e59f84924d5e549b7beaafaa624b24b3e
func_8018F2F4 title image upload  [0x8018F2F4,0x8018F468)  93 words
SHA-256 98797b654954055f64afe2c8dbd73b6e6656bf8750e60dbd9edc455886ed68d3
func_8018F468 title compositor    [0x8018F468,0x8018F7F0) 226 words
SHA-256 6f722cf555386eeed56ca586b89307f58145fc709761359a2ff0bc4a2e24a11b
func_8018F7F0 sprite blit         [0x8018F7F0,0x8018F958)  90 words
SHA-256 e4f0123a8c35eacbf219ed9726830a339e904e798508ce4532cfedea19d8ab85
func_8018F958 cursor blit         [0x8018F958,0x8018FBC0) 154 words
SHA-256 ac36c7979f939adcb2b298fb3833c258a6aab84e9254f10e5383bf21304b3c8a
func_8018FBC0 task spawn (kind)   [0x8018FBC0,0x8018FD04)  81 words
SHA-256 fe21ba8d190c86c040c75a805ba9a154aee145324b045ea1e3c3c31b5208e9ab
func_8018FD04 task spawn (kind 4) [0x8018FD04,0x8018FE1C)  70 words
SHA-256 4dab35eb88099d67f7f3fbb0c3580df34c644b5cdca6aa417c621306f486c9b5
func_8018FE1C menu spawn          [0x8018FE1C,0x80190064) 146 words
SHA-256 e70cfcb9b2d4327776ee96a9a415c850684e8eed0762a4ff51bb85817e0181a5
func_80190064 title input         [0x80190064,0x80190660) 383 words
SHA-256 9234e8564f458e8c177093ddffc94e7de900c7523e9603c85707114b68b8c217
task update/handler leaves        [0x80192F98,0x80193254) 8 functions
  0x80192F98 (20w) e339ece19be959aaf7538119ef943b76bc1810afed05949f5123120b2f8191d4
  0x80192FE8 (39w) 1b641bed974038f0ed4e35ed060d2827d9e4b7c44af35cd7e21b7f7a2471c74d
  0x80193084 (21w) 6150cf62f0fb7f42a1e51682ce4c253edaee314f82624bb14c6391be9e7bfadd
  0x801930D8 (37w) db86e9eaffeddab4d80032c1addb93a36031e8cf0ea641eaa5b5925682642cda
  0x8019316C (12w) ab38ab30a1ccb884a587944369d287f26dd9dbc970fb8d6cf509b08f84123e0b
  0x8019319C ( 8w) 35d5b997c39024fb277cd3e709dddc260c16995db976f305f7316e1952a7115b
  0x801931BC (17w) 7dee274602c8252507abb86d170fb5d85d0a5857e8163422422efb59f0fe7487
  0x80193200 (21w) 39d79d014444c540ce5ab7e6c69e680282cb89793bffa7c19300a7726a2e4774
func_8005E038 pad word remap (exe) [0x8005E038,0x8005E114)  55 words
func_8006EBD4/EBE4/EC08 movie-state readers (exe) 4/9/25 words
  faa4008d208ed6e99ac2d6a8d13b5fb2003a3e55ae0b153e3077c168c7f5d0a2
  b0b4ca87e00b6372ace9b29416e607e69eb97d5e436194f8b2abc758731a610b
  9169b21f6777fecf6a13ffb235c562b0c2b695949cb671d1e01624e50341eeed
```

## 1. Who calls the movie player, and what runs after it returns

Exactly one `jal func_801924F8` exists in the executable plus overlay:
`0x80192E00` inside `func_80192CE8`. Exactly one `jal func_80192CE8`
exists: `0x80190D84` inside `func_801909B4`, with `a0 = 1` (record index 1
= `\FMV1\FMV001.STR;1`, LBA 189742, B54K-AD). The executable text contains
no call into the overlay text range other than through the retail overlay
entry already adopted by the port.

Call chain and control flow (retail words quoted from the images):

```text
func_801909B4
  0x80190A94  lbu  $v0, D_800B0DCD          s4 = D_800B0DCD & 1 (kept in $s4)
  0x80190D7C  beqz $s4, 0x80191120         s4 == 0: no movie, straight to title
  0x80190D84  jal  func_80192CE8  a0 = 1
  0x80190D8C  beqz $v0, 0x80190DB4         v0 != 0 only when the pad skipped it:
  0x80190D98..0x80190DAC                   60 x { func_800425DC(); VSync(0); }
  [0x80190DB4,0x80191120)  copy DrawEnv/DispEnv templates into the overlay
              environment pair, ClearImage x4, FillRect of the dirty rect,
              VSync(0), ResetGraph(1), PutDrawEnv, PutDispEnv
  0x80191120  jal  func_8018F2F4           title background upload (below)
  0x80191128  VSync(0); 0x80191130 SetDispMask(1)
  [0x80191138,0x80191194)  7-node free list at 0x801D11CC (stride 0x34),
              head D_801D136C, active list D_801D1370/1374/1378/137C = 0
  0x80191198  jal  func_8018FBC0  a0 = 1   spawn task kind 1
  0x801911A8  sw   0x8019319C, 0x14(node)  its handler (spawns kind 2)
  0x801911B4  sw   $s4, D_801D1380         attract counter (+= s4 per frame)
  [0x801911C0,0x80191410)  TITLE LOOP (below)
  [0x80191410,0x80191724)  loop exit: selection processing / attract restart
  0x80191724  bltz $s2, 0x80190BCC         attract timeout re-enters the sequence
  [0x8019172C,0x801918F8)  restore env templates; return $s2 (selected mode)
```

`func_80192CE8` around the movie call:

```text
  0x80192CE8..0x80192DF0  flags |= 0x200 (D_800B0CD8), record byte = 1,
              SetDispMask(0), DrawSync, ResetGraph(1), table-selected PE.IMG
              read with retry (B54K-Y), cache critical section
  0x80192DF4  jal  func_80191FB8 (1, sp+0x10)   writes D_800B0DBA = 1,
              D_800B0DBC = 0 (translated, B54K-Z)
  0x80192E00  jal  func_801924F8   a0 = (int16)index
  0x80192E08..0x80192E1C  lbu D_800B0DBA; beqz -> 0x80192F60   ($v0 IGNORED)
  0x80192E24  lh   D_800B0DBC; blez -> 0x80192F60
  0x80192E34  jal  func_8003EB04            pad read
  0x80192E3C  jal  func_80192934            per-frame movie step (byte result)
  0x80192E48  bnez -> 0x80192E90            nonzero = still playing
  0x80192E50..0x80192E84  movie ended: zero D_801D0DE8/DEC/DFC/DF8/DF0/DF4,
              D_800B0DBA = 0 -> 0x80192F44
  0x80192E94  lw D_8009D26C; and 0x20000004; beqz -> 0x80192F44
  0x80192EA8..0x80192F20  pad skip: D_800B0DBA--, func_800870F0(0),
              func_8010C0D8(0), func_8007A2A4(), func_80080DC4(9,0,0) [pause],
              zero the six words, D_800B0DBA = 0
  0x80192F24  slti frames < 0x578 -> VSync(0); SetDispMask(0)
  0x80192F40  s3 = 1
  0x80192F44  jal  func_80070E54            display flip
  0x80192F50  lbu D_800B0DBA; bnez -> 0x80192E24
  0x80192F60  D_800B0CD8 &= ~0x200; return s3 (0 = ran to completion, 1 = skipped)
```

### Movie player return contract

`func_801924F8` returns 0 at its guard (`record_index >= 47`, B54K-AA).
Otherwise it acquires only the FIRST decoded frame: the 2000-iteration
`func_80191B64` poll at `0x801927E0`, and on success `0x80192814..0x80192860`
(frame count++, buffer toggle, `func_8010C89C`, `func_8007C394`, `v0 = 0`).
`0x80192868..0x80192874`:

```text
  80192868  sra   $v0, $v0, 0x10
  8019286c  addiu $v1, $zero, -1
  80192870  bne   $v0, $v1, 0x801928ec        taken on success (v0 == 0)
  80192874  addiu $v0, $zero, 1               delay slot: return value = 1
```

The success tail `[0x801928EC,0x80192934)` stores `D_801D0DBD = 0`,
`D_800B0DBC = 1`, `D_800B0DBA += 1`, restores registers and returns with
`$v0 = 1`. The timeout arm (`v0 == -1`) re-arms Setloc/ReadS and loops; it
never returns. **Retail's normal return value is 1, and the sole caller
does not read `$v0`** (`0x80192E08` loads `D_800B0DBA` instead). The
remaining frames are consumed by the caller's loop through
`func_80192934`, not by the player.

### Memory the caller reads after the player returns

`D_800B0DBA` (movie-active count) and `D_800B0DBC` (frames delivered).
Cross-image xref census (lui/lo16 tracked):

```text
D_800B0DBA  writers: 0x8019215C(=1 in func_80191FB8) 0x801928F0(++)
                     0x80192E84(=0) 0x80192EBC(--) 0x80192F20(=0)
            readers: 0x80192024 0x80192938 0x80192E10 0x80192EAC 0x80192F50 (overlay)
                     0x8006EBD8 0x8006EBE8 0x8006EC0C (exe: func_8006EBD4 /
                     func_8006EBE4 / func_8006EC08)
D_800B0DBC  writers: 0x80192174(=0) 0x80192848(++) 0x80192908(=1) 0x80192A9C
            readers: 0x80192820 0x80192A74 0x80192E24 (overlay)
                     0x8006EBFC 0x8006EC28 (exe)
```

Executable readers and their callers (jal census):

- `func_8006EBD4` returns `D_800B0DBA`: no static caller.
- `func_8006EBE4` returns `D_800B0DBC` when active else -1: callers
  `0x80018FE8` (field-script wait opcode) and `0x80070E8C` inside
  `func_80070E54`, but only when `D_800B0CD8 & 0x200` (movie sequence flag,
  cleared at `0x80192F70` before `func_80192CE8` returns).
- `func_8006EC08` returns 0 unless active AND frames > 0: callers
  `0x80019008`, `0x80019068` (script opcodes), `0x80036F94`, `0x8003F50C`
  (`func_8003F3C4`, translated), `0x80070EEC` (`func_80070E54`).

Consequence for a player-entry bypass that writes nothing: the caller sees
`D_800B0DBA == 1` (set by `func_80191FB8`) and `D_800B0DBC == 0`, takes the
`blez` at `0x80192E2C` to `0x80192F60`, clears bit 0x200 and returns 0
("ran to completion"). No fabricated state is required for the caller to
proceed. The residual difference from retail is `D_800B0DBA == 1` instead
of 0 and the six buffer words left non-zero; on the mapped boot path every
executable reader is either gated off (`0x200` cleared) or returns 0 for
zero frames, identical to the inactive case. The residual matters only to
`func_8006EBE4`'s field-script caller (returns 0 instead of -1), which is
not on the path to the title screen. This is recorded, not hidden.

## 2. How many movies play before the first interactive screen

One. The only movie call is `func_80192CE8(1)` at `0x80190D84`, guarded by
`D_800B0DCD & 1` (Disc 1 boot value is 1 per the adopted retail image,
B54K-R). `func_80192CE8` plays the single record it is given. A second play
happens only through the attract timeout: `D_801D1380 += s4` each title
frame, and when it reaches 1000 with no selection the exit path returns to
`0x80190BCC` and the movie sequence re-runs. The exit path also contains
`func_8005E588/E6F0/E788/E6E4` (CD/XA audio control) after a selection.

## 3. First function that waits on controller input, and what it draws

The title loop `[0x801911C0,0x80191410)` runs every frame:

```text
  0x801911C0..0x801911EC  environment swap (D_801D11C8 ^= 1, D_801D11C4)
  0x801911F0  jal func_800425DC            memory-card slot poll (exe)
  0x801911F8  jal func_8003EB04            pad read (translated)
  0x80191200  jal func_80190064            TITLE INPUT HANDLER
  0x80191208  jal func_8018F468            composite task sprites
  0x80191210..0x80191314  retire tasks whose +0x30 is set (free list)
  0x80191318  DrawSync(0)
  0x80191320..0x801913B8  LoadImage(dirty rect env+0x70, env+0x8080 buffer)
  0x801913BC  VSync(2); ResetGraph(1); PutDrawEnv(D_801D11C4); PutDispEnv(+0x5C)
  0x801913EC..0x80191408  D_801D1380 += s4; loop while < 1000
```

`func_80190064` is the first input consumer. It reads
`func_8005E038()`, the remap of `D_8009D26C` into the game pad word:

```text
D26C bit -> pad word   0x8->0x1000(up) 0x20->0x4000(down) 0x40->0x8000(left)
                       0x10->0x2000(right) 0x20000000->0x20 0x40000000->0x40
                       0x10000000->0x10 0x80000000->0x80 0x04000000->0x4
                       0x08000000->0x8 0x01000000->0x1 0x02000000->0x2
                       0x2->0x100 0x4->0x800
```

Its decisions (`D_801D11B8` = previous word, edge-detected):

- `0x800` (D26C bit 0x4, Start): if task kind 2 has faded in (`+0x1C >= 0x81`)
  and task kind 1 is idle, arm kind 1 (`+0x24 = 1`, `+0x28 = 8`, handler
  `0x801931BC`), reset the attract counter, `func_800525EC()` (SE).
- `0x1000` / `0x4000`: move the kind-5 cursor by 20 px between item rows
  `0xA0`, `0xB4`, `0xC8` (and `0x8C` when a save exists:
  `func_80042770(0|1)` slot-present, `func_8003FFCC()` valid save), SE via
  `func_8005267C()`.
- `0x20` (D26C bit 0x20000000, confirm): when the cursor row is one of
  `0xA0/0xB4/0xC8/0x8C`, `D_801D1380 = 1001` ends the loop and
  `func_800525EC()`.

What it draws: the screen is a 320x240 **24-bit** framebuffer. `func_8018F2F4`
uploads the 480x204 background (overlay data header `0x8019327C`, pixels at
`+0x14`, `w=480 (16-bit units) h=204`) to VRAM `(0,0x14)` / `(0,0x104)` for
both buffers via `func_8007506C` (LoadImage). Task sprites are 24 bpp images
in the overlay (`0x80193254` table, kind offsets at `0x80193258`):

```text
kind 1  216x24  pos (0x58,0xB4)   kind 2  162x24  pos (0x68,0xC8)
kind 3..7  126x24  x=0x74  rows 0xA0 / 0xB4 / 0xB4 / 0x8C / 0xB4
kind 8  16x1 (cursor bar)          positions from 0x801D0D5C (12 B per kind)
```

`func_8018F468` copies the background rows of the union rect into the RAM
buffer at `env + 0x8080` (stride 0xF0 words = 320 px x 3 B) and calls each
task's draw pointer (`0x8018F7F0` alpha-blend blit, `0x8018F958` cursor);
the loop then LoadImages only the dirty rect. Task kind 1's handler
`0x8019319C` spawns kind 2; kind 2 fades in through `0x80192FE8`
(`+0x1C` alpha 0..0x100). Start therefore waits for the fade before it is
accepted.

## 4. Native readiness

Translated today: `func_801909B4` through `0x80190D8C`, `func_80192CE8`
through the movie call, `func_8003EB04`, `func_8007506C`, VSync /
SetDispMask / ClearImage / PutDispEnv inlines, PutDrawEnv, ResetGraph,
DrawSync. Not yet in the port and on this path: `func_800425DC` (memory
card poll), `func_8005E038`, `func_80042770`, `func_8003FFCC`,
`func_800525EC` / `func_8005267C` (SE through `func_8006DF50`), the whole
overlay title system above, and `func_80070E54`.

```text
MOVIE_CALLER=func_80192CE8@0x80190D84(a0=1)<-func_801909B4
MOVIE_PLAYER_CALL=0x80192E00
MOVIE_PLAYER_NORMAL_RETURN=1_(ignored_by_caller)
MOVIES_BEFORE_TITLE=1_(FMV001.STR;attract_replay_after_1000_frames)
FIRST_INPUT_WAIT=func_80190064@0x80190064_in_loop_0x801911C0
TITLE_DISPLAY=320x240_24bpp_CPU_composited_LoadImage
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=flag_gated_movie_bypass_at_func_801924F8_entry
```

## 5. Phase 2 — the flag-gated bypass (same rung)

`PE_PORT_SKIP_FMV=1` (or `--skip-fmv`) is a runtime flag stored by the
run-control policy (`PE_Port_SetSkipFmv`, reset to off by
`PE_Port_RunControlReset`).  In `func_801924F8`, after the retail guard
(`sltiu $v0,$s0,0x2F` / `bnez` at `0x80192508..0x80192518`, early return 0),
the bypass records order-log entry `func_801924F8_fmv_bypass`, traces the
same name, and returns 1 — the value the success tail returns
(`0x80192874 addiu $v0,$zero,1` in the `bne` delay slot; the tail never
rewrites `$v0`).  It writes no guest memory and calls nothing.

The caller then reads `D_800B0DBA == 1` (from `func_80191FB8`) and
`D_800B0DBC == 0`, and retail's own `blez` at `0x80192E2C` leaves the frame
loop; `0x80192F60..0x80192F74` clears flag 0x200 and returns 0.  The
residual (`D_800B0DBA` left at 1 instead of retail's 0; the six movie buffer
words not zeroed) is documented in section 1; no code on the mapped title
path depends on it.

Frontier names live in `pc_port/configs/frontier.json` only:

```text
default   func_80081314_func_8007F0C8_cut  called from func_80081314  (flag off)
skip_fmv  func_800425DC                    called from func_801909B4  (flag on, after B54K-AS)
```

Every `b54k*.py` oracle that previously hardcoded the frontier
(b54kae/af/ag/ah/ai/aj/ak/al/am) now reads it through
`pc_port/tools/pe_frontier.py`; their expected values are unchanged.
Oracle: `pc_port/tools/b54kaq_fmv_bypass_oracle.py`; tests
`B54KAQ_fmv_bypass_flag_on` / `_flag_off`.

```text
FMV_BYPASS=PE_PORT_SKIP_FMV_runtime_flag_only
FMV_BYPASS_RETURN=1_retail_success_tail_value
FMV_BYPASS_MEMORY_WRITES=none
FLAG_OFF_FRONTIER=func_80081314_func_8007F0C8_cut_unchanged
```
