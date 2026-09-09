# Complete opening movie frame verification

Stage153 tests the first three complete frames of FMV001.STR from the user's
Disc1 image. This adds real compressed frame coverage to the earlier synthetic
VLC vectors and synthetic two-macroblock DMA integration. It does not yet wire
the movie player, prove stream timing, or establish hardware-exact pixels.

`pe_movie_complete_frame_oracle.py` authenticates the original executable and
libpress overlay, executes the original table builder at8010BD4C, then executes
8010C89C on complete frames. The table spans69632 bytes. Video chunk headers
are validated for type80010160, sequence, count, frame number, compressed size,
width and height; duplicated sector subheaders must agree. Interleaved audio
sectors are excluded. Each video chunk contributes2016 bytes from raw offset56.
The padded input is hashed in full; no game frame payload is checked in.

| Frame | First–last LBA | Video chunks | Compressed bytes | RLE bytes including command |
| --- | --- | --- | --- | --- |
| 1 | 189742–189751 | 9 | 2712 | 7300 |
| 2 | 189752–189761 | 9 | 3628 | 11140 |
| 3 | 189762–189771 | 9 | 4356 | 12804 |

All three frames are320×240. Original decoding reaches the normal pad exit,
consumes exactly the declared compressed byte count, writes through exactly
the command's declared RLE extent, and preserves a16-byte output canary.
COP0 Status instructions at8010CBAC/CBBC are explicit read/write providers:
read returns0, original arithmetic produces20000hex, and write is consumed.
This verifies the RAM transformation and return, not CPU cache/status fidelity.
No original instructions are patched. Native comments were corrected: the observed OR is bit17, not the previously
claimed IEc bit0.

`test_movie_complete_frame.h` reads the actual user disc and builds the tables
with the native port. It checks the complete table and padded input hashes,
then compares every RLE output byte through its hash against the original
instruction graph and checks the output canary. Each frame subsequently passes
through actual libpress reset/input/output calls and the MDEC owner, requesting
300 macroblocks in both15-bit and24-bit modes. This verifies that real frame
RLE can traverse the numerical pipeline and complete each DMA request. It
has no captured hardware pixel golden, no GPU upload, and no movie callback;
those facts must not be inferred from a passing test.

The existing `pe_movie_frame_oracle.py` covers display setup/frame acquisition
and remains separate. During development a filename collision overwrote that
script; it was restored from its exact prior creation command in the session
record. Its `--check` again passes140 display and122 acquisition graphs against
the unchanged existing fixture. The first native complete-frame attempt used
a legacy FNV helper whose offset basis omits a digit. The fixture was corrected
to use the existing standard MDEC FNV helper; expected frame hashes were not
changed to accommodate the failure.

Remaining work includes live sector assembly into player-owned buffers,
121C04/122040 and14E30 integration, the old80191DC8 callback, command/status
wait fidelity, XA mode/filter/audio handling, hardware pixel validation, and
opening-through-Day2 acceptance. Three frames are a bounded data-path check,
not proof that a movie or any entire day is complete.

Validation: original complete-frame oracle and legacy frame oracle `--check`
pass; Python compilation and whitespace checks pass. Normal and ASan/UBSan
builds complete without warnings, and each focused run passes45 groups with
1297 skipped. Full normal CTest passes8/8 in133.65s, including1342/1342 native
groups with0 skipped. Logs are `local/live/*day2-153*.log`. All jobs completed.
