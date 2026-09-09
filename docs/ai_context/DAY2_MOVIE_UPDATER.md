# Movie updater and stream dependencies

Stage157 ports and validates the movie player 80121C04 (271 words,
`local/live/movie-player-152.asm`). The player selects the movie record by
halfword id (>= 0x2F returns 0), stores B0DBF/B0DBB and the record pointer
227E4, configures both display environments through 21004, builds the STR
filename by strcat (prefix table word at 120FF4/120FFC by id<0x15, then the
record's word 0), resolves it with 81414 (DsSearchFile) behind the readiness
gate, copies 223FC to 22414, seeds the slice geometry (record+0xA x, +0xC y
+0xF0, bank byte from CDDC, width 24/16 by B0DBB), initializes libpress
(BE3C(0)), registers the 1214D4 output callback, opens the record pool
(A214(2434,0x40)) and the streaming read (C304(1, record+6 LBA, -1, 0)),
then Setloc/81314(0x1E0) behind readiness, 870F0(B0DBE), BD4C(2430) and the
first-frame decode loop (121270 -> flip 228D4/B0DBC -> C89C -> C394 ->
23F5=0/B0DBA++/B0DBC=1, return 1 through the bne delay slot).

`pc_port/game/boot/movie_overlay_port.c` implements it; the three
readiness/Setloc retry loops keep the recorded boundaries visible with the
same host-safety cycle bounds as the updater (movie_player_search_wait,
movie_player_start_wait, movie_retry_wait). Oracle
`pe_movie_player_oracle.py` passes 20 original graphs (4 early returns,
16 full graphs including a failed-first search retry); native group
DAY2_movie_player: early returns, the disabled-device search boundary with
the full setup prefix checked, and the enabled-device path where the real
fixture ISO search resolves, the start Setloc completes through the
stage155 queue, the streaming callbacks install (B8AB4=813E8, DMA3
slot=7C214), and the mode-0x1E0 ReadS reaches the device model's recorded
CD_device_read_mode boundary (read modes with 0x50 bits are the recorded
XA-mode frontier). BD4C's libpress-module source is seeded with the FF FF
terminator in the fixture; the real table build is covered by
DAY2_movie_complete_frame. The port bug found during bring-up: the player's
prefix-table strcat must LOAD the table word (120FF4/120FFC) before use.

Stage156 validates the complete updater 80122040 and abort helper 80122354
natively. The updater, the abort helper, the restore 121A00 and the field
presentation restore 6E60C are ported in `pc_port/game/boot/movie_overlay_port.c`,
and the game-loop branch that selects them (3F51C in `func_8003F3C4_port.c`:
6EC08 low byte selects 122040; low-byte return 0 continues through 121A00 and
6E60C to the loop tail, nonzero goes to the 70E54 frame tail) is wired. Six
SDK helpers used by retry/teardown are native in `cd_stream_port.c`, with
their declarations in `pe_sdk.h`; the stage155 command queue gives the real
Setloc/ReadS/Pause issue path.

Original control flow (authority `pe_movie_update_oracle.py`, 192 graphs with
explicit SDK/frame/timeout-counter providers, `--check` PASS):

1. If unsigned byteB0DBA is less than2, return0. If223F8 equals2, configure the
   opposite display bank using the signed low byte of9CDDC xor1 and signed
   B0DBB, then clear223F8. Copy the four location bytes223FC→22414.
2. Select the current RLE arena through byte228D4 and table228CC; submit it
   throughBFA0 with mode byte223F6. Submit the current output buffer selected
   by228E0 through table228D8. Output words are signed(slice width×height)/2,
   rounded toward zero, from halfwords228F8/228FA.
3. Acquire the next compressed frame through121270(state228CC), retrying2000
   times. On success, flip byte228D4, increment halfwordB0DBC, runC89C with the
   opposite RLE arena and table pointer22430, then release the stream record
   through7C394. This prepares the next frame while the current frame's MDEC
   output and DMA callbacks proceed.
4. On acquisition timeout, call7C2A0(location22414). Wait until7F72C returns1
   and7F778 returns0. Issue Setloc through80D5C(command2, location22414,
   stack response), then81314(location22414,mode1E0). Failed issue loops back
   to readiness; success retries acquisition. The location helper's return is
   ignored. The existing collapsed80D5C rejects non-null response arguments,
   so this retry branch cannot yet be wired faithfully.
5. After preparing the next frame, wait for byte228FC with counter800000hex.
   If the counter expires, set228FC=1, toggle byte228F2, and restore slice x/y
   from the selected frame rectangle at228E2/+2. This timeout branch is real
   original behavior; it is not evidence that pixels completed successfully.
6. Clear228FC. If223F5 is not exactly1, return1. Otherwise decrementB0DBA,
   unregister the MDEC callback withC0D8(0), call7A2A4, issue Pause through
   80DC4(9,0,0), and return0. The separate22354 abort helper first decrements
   B0DBA and calls870F0(0), then performs the same teardown/Pause sequence.

The asynchronous wait needs deliberate device/IRQ service in the native
port: original CPU execution lets DMA and callbacks progress, whereas a plain
C busy loop does not. Current HostFB query ticks provide an approximate
service checkpoint. Cycle-accurate timing and CPU status/cache behavior are
not established. The original retry can also wait indefinitely; a native
boundary must remain visible when no modeled provider can make progress.

## Stage156 native validation

`pc_port/tests/test_movie_updater.h` (group DAY2_movie_updater) executes the
real SDK/MDEC/CD implementations against the updater:

- B0DBA<2 early return: return 0, no calls, no mutation.
- Full continue path (device disabled, end flag 0): return 1; B0DBA kept,
  B0DBC wrapped 65535->0, 228D4 flipped, 22414 keeps the copied location
  (proves 7C2A0 was not reached), C89C telemetry (entry triple and bound
  exit 1), MDEC upload (mode-3 edited command word 0x62000100, source
  arena+4) and output request (MADR masked 24-bit, BCR 0x5A0020 for the
  24x240 slice), the 0x800000 countdown expiry arm (F2 toggle plus the
  slice x/y restore from the frame rectangle), and the 223F8=2 bank
  reconfigure through 21004 (223F6=3, disp+17 wide flag).
- Retry boundary: with the device disabled the readiness/Setloc retry
  stops at the explicit movie_retry_wait boundary after the recorded
  2000-poll exhaustion; the 7C2A0 position write (0x11010200 from
  A3490=0x200 plus the destination's retained fourth byte) and all
  pre-stop state are checked. The enabled-device completion of that
  branch (real Setloc/ReadS through the stage155 queue) is NOT exercised
  yet; driving it requires the autonomous physical stream so 121270 can
  eventually succeed, which is the next integration frontier.
- Abort helper 22354: decrement, 870F0 sound stop through the CD pointer
  table, MDEC callback unregister, stream teardown bank clears, then the
  device-disabled Pause stop at the recorded CD_command_wait boundary.
- Enabled-device teardown: with the stage155 disc fixture and startup the
  Pause completes through the real queue, the updater returns 0, and the
  queue retirement and teardown bank clears are checked.

## Stage156 GPU DMA2 completion during the movie wait

The complete-frame test (DAY2_movie_complete_frame, real FMV001.STR frames
run through the actual updater and the 1214D4 slice callback) exposed a
timing-model gap: GPU DMA2 movie-slice uploads completed only at the
explicit main-loop/DrawSync checkpoints, so during the updater's wait the
queued LoadImages drained late and read slice buffers the MDEC had already
refilled (a whole column of the frame landed 13 slices late). Hardware
advances the GPU DMA and the MDEC in parallel. `HostFB_VSync` now runs the
existing `PE_Port_ServiceDmaIrqCheckpoint` before `PE_MDEC_Service`, but
only when a decode is in flight AND a GPU DMA2 transfer is pending; with no
pending transfer the tick ordering is unchanged (DAY2_mdec_dma's masked-IRQ
step granularity is preserved). With this, the complete-frame test's VRAM
comparison of all 115200 pixels per real frame matches in both movie
formats. Model pixel rounding remains model output, not a hardware golden.

## Helpers ported in stage154

`7A930..7AA34` converts a signed LBA to three BCD bytes after wrapping LBA+150.
It uses signed divisions and writes frame, second, then minute, leaving the
fourth byte intact and returning the destination. `7AA34..7AAB4` performs the
inverse arithmetic without validating BCD digits. These cannot blindly reuse
the existing unsigned positive-LBA conversion accommodation.

`7C2A0..7C2F8` returns-1 without mutation whenA8020 is nonzero. Otherwise it
converts the last completed stream location atA3490 to LBA, advances one sector,
and writes the caller's location. It then returns the live word atA3494;
overlapping destinations can change this result. That read must follow stores.

`7A4BC..7A4D0` exchanges low-level data callback9AFB8. `7A8EC..7A910` forwards
registration to DMA slot3 through73CF4. `7A2A4..7A324` enters the critical
section, then selects teardown by9AFD8==1: clear DMA3 through824F0 and high-level
data callbackB8AB4 through824C8; otherwise clear DMA3 through7A8EC and low-level
callback9AFB8 through7A4BC. It next writes0 through9AF1C, then0 through9AF28,
then exits the critical section. Register pointers support guest RAM or the
existing physical CD owner; no callback return or device response is fabricated.

`pe_movie_stream_helpers_oracle.py` executes20 signed conversion cases,
8 inverse cases including invalid BCD digits,120 complete retry graphs with
nonzero guards and five destination alias patterns, and8 teardown graphs.
Teardown critical-section and DMA-registration calls are explicit providers;
the callback-layer selection, RAM stores, call order and arguments execute
from original instructions. Native tests execute actual SDK critical-section
and DMA-registration implementations, compare original results, verify slot3
unregistered, and add physical CD-register bank clearing.

The remaining command dependency is larger than a Pause shortcut: original
80DC4 and80D5C have identical command-queue issue/poll structure, using7EE84
and7F418. 7EE84 is145 words through7F0C8;7F418 is124 words through7F608. Local
inspection files are `local/live/movie-command-*-154.asm`; the7F418 carve was
extended through its return (with the next function prefix). Full queue and
response semantics are required for both Setloc retry and Pause teardown.
Player121C04,14E30 integration, old80191DC8 callback, XA handling and complete
opening-through-Day2 acceptance remain open.

Validation (both stages): original oracles `--check`, Python compilation and
whitespace checks pass. Normal and ASan/UBSan builds are warning-free.
Focused DAY2 runs are terminal0 in both builds, each49 PASS/1297 skip/1346
total (logs `local/live/test-day2-157.log`, `test-asan-day2-157.log`); full
normal CTest passes8/8 in140.74s including1346/1346 native groups with0
skipped (log `local/live/ctest-day2-157.log`); the full ASan binary
separately passes 1346/1346. All stage156/157 jobs completed.
