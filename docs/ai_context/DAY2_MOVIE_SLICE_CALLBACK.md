# Movie slice callback: translated control flow with two open callees

Stage135 translates124 original words at801214D4..801216C4 into
`pc_port/game/boot/movie_overlay_port.c`. Original controller package is
LBA1978/4 sectors, SHA256
5ddd18d8a7f2a8180f92c1c9c072996e9705605e2daac8dc02a665a35f470ec0.
Callback byte-span SHA256:
ed8d32b27e7d9a0be164b335a9bd0195625433bc57dd4d8bfee21ecfc1819ad1.

The native callback is PARTIAL: it stops at8007C564 when a signed nonzero
movie format and pending halfwordB0CD0 select that stream handler. It stops
at8010C01C when another decoded slice must be requested. Neither boundary
supplies a fake return or decoded pixels. The original continuation after
7C564 clearsB0CD0; this is deliberately not executed before that callee exists.
The original continuation afterC01C uploads the completed previous buffer;
this is also not executed after an unresolved output request. These guards
must be replaced with real calls and stop propagation when dependencies land.

Implemented behavior after the stream gate:

- Save the current8-byte rectangle before modifying frame state.
- Toggle the unsigned current buffer, advance rectangle X with16-bit wrapping,
  and compare its signed result with signed bank X plus signed bank width.
- On a continuing slice, preserve the exact next-buffer pointer and signed
  width*height/2 (truncate toward zero) arguments at the output boundary.
- On the final slice, set1228FC, toggle frame bank, and reload its X/Y.
- When1223F8 equals1, XOR the raw format byte, choose slice width24 or16,
  call the real121004 display setup using signed low bytes and8009CDDC,
  and advance the flag to2. Other flag values are retained.
- Upload the saved rectangle from the previous buffer through the real
  8007506C/GPU path. The new rectangle and new format must not replace the
  saved upload rectangle. The previous buffer lookup uses signed lowbyte
  indexing, unlike the next-buffer lookup.

`pc_port/tools/pe_movie_callback_oracle.py` authenticates the executable and
controller overlay, executes360 original callback graphs and emits
`retail_movie_callback_cases.h`. Stream8007C564, output8010C01C and GPU8007506C
are explicit providers. Their order and output/upload arguments are checked;
providers return0 without pretending to execute hardware. Display setup and
its environment constructors execute original instructions.

The oracle records both the RAM snapshot at the first unported callee and
RAM after the complete original provider graph. The native test compares
84 complete final-slice paths and276 prefixes ending at an explicit boundary.
Complete paths check every uploaded pixel from a distinguishable source
buffer and adjacent untouched pixels after completing the scheduled GPU DMA
through the existing blocking DrawSync path. The first fixture incorrectly
inspected pixels before that asynchronous completion; only the test changed.
Boundary paths check state, named
stop, exact output arguments where applicable, and absence of stale uploads.
Coverage varies raw format0/1/2/128/255, pending halfword0/1/8000, both buffer
indices, both frame banks, flag0/1/2 and continuing/final slices. It does not
claim invalid buffer-index, signed-coordinate overflow or full playback coverage.

Next dependencies remain substantial.8007C564 spans8007C564..8007CE80
(583 words), calls7A488,7CE80,7CEAC,7C444,7C214 and two indirect callbacks.
Its raw disassembly is local/live/movie-stream-handler-135.asm; that initial
inspection includes the first instruction of the following7CE80 helper.
The existing MDEC model only records reset/table submissions. Actual RLE,
IDCT, color conversion, output DMA and interrupt delivery remain unimplemented.
No third-party decoder code was imported. The consulted
[MDEC hardware reference](https://psx-spx.consoledev.net/macroblockdecodermdec/)
leaves rounding details uncertain; a future decoder needs pixel evidence,
not just successful transfer bookkeeping.

Player121C04, updater122040, loader14E30, opening and complete Day1/Day2
acceptance remain open. The game loop is not wired to this partial callback,
and no package or release is published by this stage. Final checks are
recorded in ACTIVE_HANDOFF.md.

Final focused normal and ASan/UBSan runs each pass1group with1324skipped;
all360 cases execute. Both builds are warning-free. Original fixture
regeneration/check, Python compilation and scoped whitespace checks pass.
Full normal CTest passes8/8 in146.12s, including1325/1325 native groups
with0skipped. Log: local/live/ctest-day2-135.log.

Stage136 supersedes the stream-call boundary described above: the callback
now calls7C564, propagates its stops and clearsB0CD0 after successful return.
The current360-case callback oracle executes the real stream active guard,
with180 complete final-slice paths and180 output prefixes. The stage135
84/276 figures are historical. See DAY2_STREAM_RECORD_ASSEMBLY.md.
