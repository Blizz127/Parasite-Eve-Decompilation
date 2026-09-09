# Movie display setup and frame acquisition

Stage134 extends the native movie overlay with121004..121270 (155 words)
and121270..1214D4 (153 words). Source is the stage133 pinned movie-controller
package at LBA1978/4 sectors, guest base80120D00; raw inspection remains local.
Both functions are in `pc_port/game/boot/movie_overlay_port.c`. The full movie
player, updater, DMA callback and14E30 integration are still unfinished.

Display setup uses the signed low byte of the buffer index for20-byte display
and92-byte draw environment strides. Low-byte buffer0 displays atY240 and
draws atY0; other buffers display atY0 and draw atY240. The full original
SetDefDispEnv/SetDefDrawEnv functions execute, including reserved-byte behavior.
Low-byte format0 sets320x240 and overlay byte1223F6=2. Other formats select
480x240, set display24-bit flag, and reduce both widths to320 via signed2/3;
1223F6=3. Both paths explicitly set the original draw flags/background bytes.
These are environment settings, not evidence of a decoded24-bit movie frame.

Frame acquisition121270 has the following order:

1. Read CD readiness; when lane1 has a changed status value, call719E4(1).
2. Poll7C484 with two output addresses up to2000 times. A successful poll
   promotes state2 to4 and publishes actual data/header addresses. Timeout
   returns0 without manufacturing a frame.
3. Compare unsigned frame number against signed-record-limit-minus16. Near
   the limit, compute the original clamped factor and signed multiply-high
   scale, then call870F0 to set volume.
4. Set completion flag1223F5 when frame regresses relative to sign-extended
   last-frame halfword, or reaches/exceeds sign-extended record limit. Store
   the current frame's low half at1227E8. This flag is not a completed movie.
5. Compare incoming unsigned width/height to signed cached halfwords. If
   different, clear(0,0,320,480) or(0,0,480,480), then refresh cached dimensions.
6. Always publish widths to state+26/+34, even when dimensions were unchanged.
   The24-bit path uses the original signed-width*3/2 low-halfword operation.
   Publish height to state+28/+36/+46 and return the actual data address.

The output addresses use explicit guest scratch cells801FFF20/24, matching
the existing host frame-poll convention. This is a host stack adaptation,
not original stack-residue equivalence. Tests use readiness lane2 and do not
prove the changed-status719E4/BIOS path or asynchronous CD delivery. Native
propagates unresolved provider stops instead of proceeding as if successful.

`pe_movie_frame_oracle.py` verifies original executable/overlay hashes and
produces `retail_movie_frame_cases.h`. It executes140 complete original display
graphs and122 frame graphs. Display cases vary signed buffer indices,
low-byte aliases, format, video standard and initial bytes. Frame cases vary
format, dimensions (including signed-boundary values), unchanged dimensions,
frame regression/end boundaries, negative limits and empty-pool timeout.

Original frame tests run the real readiness getter, record poller and volume
arithmetic. ClearImage and7A88C are explicit GPU/audio provider boundaries:
arguments are checked and return0 is supplied. Native tests execute actual
GPU clears and the existing audio adapter, compare original RAM/returns and
check clear counts and inside/outside pixels. This is not proof of original
GPU/audio hardware or complete playback. Native display comparisons execute
the real environment constructors on both sides without provider substitution.

Remaining work: movie DMA callback1214D4, full player121C04, updater122040,
SDK/DMA output delivery and actual decoding/playback. The opening and complete
Day1/Day2 path must subsequently be verified without treating movie skips,
closed handler tests or static routes as completed scenes.

The isolated native fixture seeds the original four CD-register pointers
(9B27C/280/284/288 → 1F801800/1/2/3). Its first run omitted this prerequisite
and aborted on address0 during volume application; the fixture was corrected
without changing the production path. Final results are in ACTIVE_HANDOFF.md.

ClearImage's width320 path uses a GP0 fill; width480 uses the original
mono-rectangle path because its width is not64-aligned. The native assertion
checks the corresponding packet counts and pixels; an initial assertion
incorrectly counted both paths as quick fills. Production behavior was retained.

Final focused validation: normal and ASan/UBSan each pass1group with1323
skipped; all140 display and122 frame cases execute. Builds are warning-free;
original fixture regeneration, Python compilation and whitespace checks pass.
Full normal CTest passes8/8 in169.25s, including1324/1324 native groups
with0 skipped; log local/live/ctest-day2-134.log.
