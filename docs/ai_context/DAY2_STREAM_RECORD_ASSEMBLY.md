# Stream record assembly and status poll

Stage136 ports the executable's stream record assembly into
`pc_port/game/boot/cd_stream_port.c` and connects it to the movie slice callback.
The RAM-backed sector path now copies actual sector bytes, validates headers,
handles record-pool wrapping and publishes completed records. The physical
CD path and movie pixel decoding remain unfinished.

Original executable SHA1 is452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
Source spans, word counts and SHA256:

| Span | Words | SHA256 |
| --- | ---: | --- |
| 7B290..7B558 | 178 | 972a1d4b2bf62126dbe837444d28a587a46884e4e31db2ddd9f048ea94e80cc1 |
| 7A488..7A4A8 | 8 | 7b9c553ab7d0abd0aaa5f09b25bb1fab2df552a245fb7fad110871528cafe0b0 |
| 7C564..7CE80 | 583 | 7c4ca8096a8921313e5b9a5f036f2b9341d656633afed2132b62af8d40aa8c74 |
| 7CE80..7CEAC | 11 | f91cd87bc431cf1eaa8877002a465f52cc61dff482723f5c3e2eedcca4bf23d4 |

7B290 establishes the VSync deadline and poll counter, prefers the queued
status at9B296 over9B295, clears only the consumed byte, copies its eight-byte
response when a destination exists, and returns0 when a nonblocking query
has no result. Its interrupt-poll7AAB4, timeout diagnostic and unknown
callbacks stop at their original calls; their unavailable results do not
permit later mutations.7A488 is the original forwarding wrapper.

The stream handler preserves the following behavior:

- Active guard equals1 exactly. MDEC output busy defers work, setsB0CD0 and
  the status word, and advances the RAM-sector index where applicable.
- Poll error/response handling and occupied-record rejection.
- CD index/request and timing-control writes, followed by copying the
  incoming eight-word header fromC0DB8+(BCD7C<<11).
- Start-frame filtering, header magic/channel checks, signed expected-part
  comparison, and partial-frame reset using only the first word of each slot.
- Frame limit callbacks, insufficient-space markers, rejection of an occupied
  slot0, and forward header copying to slot0 on an allowed wrap.
-2016-byte payload copying after the32-byte sector header, accumulation of
  part indices, final-part channel/state reset, and the existing7C214 record
  publication on the RAM path. Original copies run forward word by word.

The native movie callback now calls7C564 and clearsB0CD0's low half only after
it returns without stopping. MDEC output10C01C remains an explicit boundary.
The main player/updater/game-loop path has not been wired around these gaps.

Hardware and stack limits remain explicit. Native status uses the existing
VSync counter adapter. CD bus control at1F801018 and DMA3 register storage
at1F8010B0..BB now share the existingpe_cdreg authority, with reset and
little-endian access. Register storage does not execute DMA or synthesize
completion. A busy DMA3 wait stops; physical FIFO reads stop instead of
repeating a shadowed byte as sector data.7CEAC DMA submission is unported.
MDEC busy reads usepe_mdec's DMA1 register owner for the physical address;
redirected guest pointers retain normal RAM reads.

Original response/location locals use documented guest scratch801FFE80..8B
in the native implementation. In particular, whenA8020 is nonzero the
original stores a retained stack location word into record+28 without
reading fresh location bytes. The tests seed corresponding original stack
and native scratch values. This is a host-stack adaptation, not proof of
arbitrary original stack residue or asynchronous nested-call equivalence.

`pe_cd_stream_oracle.py` authenticates the original executable, runs56 complete
RAM/early-return stream graphs,10 prefixes at unresolved calls, and48 complete
status-poll graphs. Original CPU word-copy and record-publication routines
execute; no provider supplies payload bytes. VSync returns0 as a provider,
and the interpreter redirects MMIO pointer entries into a RAM register area.
Native tests use physical CD register addresses and compare their values
separately, plus the original RAM digest including the full20KiB record pool
and payload region. Busy-MDEC fixtures redirect that pointer on both sides;
idle cases read the native MDEC owner. Tests cover distinct payload patterns,
record and frame rejection, final/intermediate parts, wrapping and named
stops. They do not prove live CD timing, FIFO/DMA execution, interrupt-poll
implementation, empty blocking-poll timeout or malformed address safety.

The updated callback oracle runs the actual original7C564 active guard rather
than a stream-call provider. Its360 cases now contain180 native-complete
final-slice paths and180 prefixes ending at MDEC output. Separate stream tests
exercise real RAM assembly; these are not an end-to-end movie playthrough.

Final focused normal and ASan/UBSan tests each pass29 DAY2 groups with1297
skipped, including all new stream/status and callback cases. Builds are
warning-free. Full CTest passes8/8 in90.66s, including1326/1326 native groups
with0skipped. Original fixture checks, Python compilation and scoped
whitespace checks pass. Logs: local/live/*-day2-136.log.
