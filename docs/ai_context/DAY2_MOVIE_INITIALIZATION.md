# Movie overlay initialization

Stage133 extracts both original overlay packages from the verified Disc1 image:

| Package | LBA / sectors | Guest base | SHA256 |
| --- | --- | --- | --- |
| Decoder/SDK | 1940 /38 | 8010BCF8 | d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40 |
| Movie controller | 1978 /4 | 80120D00 | 5ddd18d8a7f2a8180f92c1c9c072996e9705605e2daac8dc02a665a35f470ec0 |

The executable's original9315E table is927/965/969; PE.IMG starts at1013.
Raw package bytes and inspection disassembly remain ignored in local/live.
New `movie_overlay_port.c` restores1216C4..121A00,121A00..121C04 and1223A8..1223F4. The movie
loader remains an explicit boundary until the player graph is restored.

Initializer1216C4 accepts counts whose low byte is1 or2. It validates the
required nonzero buffer pointers before checking the already-active byteB0DBA.
Invalid descriptors or an active player return0 without initializing state.
With one bufferA, it assigns:

| Global | Value |
| --- | --- |
| 122420 | A |
| 122424 | A+FA00 |
| 122434 | A+1F400 |
| 122430 | A+3F400 |
| 122428 | A+50400 |
| 12242C | A+53100 |

With two buffersA/B, the first three values remain the same;122430=B,
122428=B+11000,12242C=B+13D00. Unsigned address arithmetic preserves wrap.
It sets activeB0DBA=1, byteB0DBE=98, halfwordB0DBC=0, halfword1227E8=FFFF;
backs up two20-byte display environments fromBCE80 to1227EC and two92-byte
draw environments fromBCDC8 to122814; then invokes the real MoveImage wrapper:

1. Source(320,0,192,256) to(512,0).
2. Unless stream flag08000000 is set, source(0,448,320,64) to(512,256).

Returns1 after those calls. Native propagates an unresolved GPU stop instead
of reporting success; successful paths use existing7512C/76C34/GPU providers.
Completion helper1223A8 writes1 to1223F8 if low-byte(mode) is nonzero and
signed byteB0DBB iszero, or mode low-byte iszero andB0DBB is nonzero. Otherwise
it preserves the previous flag. This does not itself decode or finish a movie.

`pe_movie_init_oracle.py` verifies the executable/package hashes and generates
45 original initialization/restoration pairs and126 complete flag-helper runs. Original
initializer/restorer instructions execute through return; MoveImage is a
provider recording rectangle arguments and returning0. Restoration also uses
explicit DrawSync/VSync/display-mask providers, recording their order/arguments. This verifies the
initializer/restorer RAM and call contract, not original GPU execution. Native tests
compare the RAM fingerprints and returns and run actual native MoveImage;
they verify every pixel of both backup rectangles, then erase both source
rectangles and verify every restored pixel and copy count. Cases
include low-byte aliases257/258, invalid counts/pointers, already-active state,
and both stream-flag branches. Numeric fixtures contain no movie content.

Movie controller121C04 remains unported. It accepts movie IDs0..46 and indexes
20-byte records at122438. The loader's original signed16-bit argument is
revalidated by that player's unsigned16-bit range check. Inspected IDs8/9/10
refer to FMV007.STR/FMV008.STR/FMV009.STR respectively. ID7 is FMV006C.STR, so numerical
movie IDs are not identical to filename suffixes. These records do not prove
playback, audio, frame timing or completion of either day.

Remaining: full player121C04, decoder/SDK entry
points and DMA callbacks, actual media reads and playback, then14E30 wiring
and opening/Day1/Day2 scene acceptance. The full goal remains unfinished.

Restorer121A00 returns immediately if inactive. Otherwise it zeros the six
buffer pointers and active flag, calls DrawSync(0), VSync(0), SetDispMask(0),
restores both saved environments and copies VRAM back from(512,0) to(320,0).
Unless the live stream flag08000000 is set, it also restores(512,256) to
(0,448). It does not re-enable display; subsequent original code owns that.
The tests overwrite the working environments before restoration to prove
that saved state, rather than unchanged original bytes, is being restored.

Initial initializer-only validation: normal and ASan/UBSan focused tests1/1;
full CTest8/8 in109.74s. An earlier filtered run used the pre-link binary and
skipped every test; it was not counted as validation. The first sanitizer run
hit sandbox LeakSanitizer/ptrace failure; after permissions changed, the same
sanitizer test completed successfully. Restorer-inclusive builds/checks are
recorded separately in ACTIVE_HANDOFF.md; do not reuse the earlier full-suite
result as evidence for later source changes.

Restorer-inclusive focused validation: normal and ASan/UBSan both pass the
new group (1323 total,1322 skipped). Build logs contain no warnings. Oracle
regeneration check, Python compilation and scoped whitespace checks pass.
Final full normal CTest passes8/8 in109.00s (ctest-day2-133-final.log).
