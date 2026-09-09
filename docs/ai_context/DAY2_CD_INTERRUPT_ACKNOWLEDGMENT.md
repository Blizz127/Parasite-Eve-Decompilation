# CD interrupt acknowledgment and response bytes

Stage137 replaces the7AAB4 placeholder with its original interrupt-processing
control flow. Span8007AAB4..8007B010 contains343 words, not the351 count in
older placeholder comments. Executable SHA1:
452fb033f2eaa4b18aa20a5bca60b8125af3a37b. Span SHA256:
4bb9e7f87377f552d32e77cea94387bfd294c03225e7e41491c8b2a758ca77fa.
Original source: asm/disc1/6B130.s. Native source: pe_libcd.c.

The worker selects CD register bank1, samples the interrupt tag until stable,
reads at most eight available response bytes and zero-pads the remainder.
It acknowledges the interrupt and writes the interrupt mask, then applies
the original command-table gates and status-byte logic. A rising response
bit10 increments9AFCC with32-bit wrapping. Data responses with exactly one
byte suppress the status error test when selecting the pending data status;
they still publish the original status bytes and edge counter.

The actual guest jump table at11B8C is read, not replaced with an assumption
about its contents. Its original labels route as follows:

| Original label | Published state | Return |
| --- | --- | ---: |
| 7AD10 | command status5,3 or2; response atA3460 | 2 or1 |
| 7AE10 | command status5 or2; response atA3460 | 2 |
| 7AE5C | data status5 or1; response atA3468; index/request clear | 4 |
| 7AEDC | both data statuses4; responses atA3470/A3468 | 4 |
| 7AF5C | data/command statuses5; responses atA3460/A3468 | 6 |

Debug error output, unsupported tags and unknown jump targets stop at their
original diagnostic/indirect call sites. This is not a claim that those
callees execute. The existing7B010 and7B558 acknowledge loops now use CD
address-decoding access for the physical register pointer and propagate
acknowledge stops before consuming a result.

pe_cdreg now accepts an explicit device response throughPushResponse, with
up to16 bytes, distinct byte reads, response-ready indication and bank1
interrupt acknowledgment/mask storage. Unread or pending responses cannot
be overwritten by another ingress call. This API does not issue a disc
command, read sector data, assert a CPU interrupt or synthesize responses.
Legacy shadow behavior remains until response ingress is activated; other
CD status flags, command/parameter FIFOs and device timing are unfinished.
Response semantics apply to the byte accesses used by the worker; existing
word accesses to bus-control/mailbox/DMA storage remain value-only.
The hardware register semantics were checked against the
[CD controller reference](https://psx-spx.consoledev.net/cdromdrive/).

`pe_cd_ack_oracle.py` authenticates the executable and executes505 original
graphs/prefixes:501 complete cases and4 diagnostic/unknown-target prefixes.
An instruction-stop adapter provides changing FIFO-ready/read values and
acknowledgment, rather than substituting for the whole function. Native
uses the response FIFO. RAM digests cover status words, the edge counter,
three pending status bytes and all three eight-byte response arrays. Cases
vary interrupt tag, response length0/1/2/8/12, status flags, previous status,
command tables and diagnostics. Native checks also drain retained bytes
past the worker's eight-byte limit and reject an attempted overwrite.
A nested7B290 test consumes a real data response and proves it is not replayed
on a second poll. Rapidly changing interrupt-tag timing is not verified.

The stream oracle now executes the original idle7AAB4 instead of stopping
there. Its66 stream cases contain58 complete paths and8 unresolved prefixes,
plus48 status-poll cases. The first combined native run exposed a missing
B288 pointer in the old isolated stream fixture. The fixture now seeds all
four physical CD pointers, and the original interpreter explicitly redirects
all four. Production behavior was retained; regenerated expected RAM matches.

No physical sector FIFO/DMA, MDEC pixel decode/output/IRQ, full movie player,
updater or loader integration is completed by this change. Opening, Day1 and
Day2 acceptance remain unverified. Full final checks are in ACTIVE_HANDOFF.md.

Final focused normal and ASan/UBSan each pass30 DAY2 groups with1297skipped.
Final3 builds are warning-free. Original fixture regeneration checks, Python
compilation and scoped whitespace pass. Full CTest passes8/8 in91.32s,
including1327/1327 native groups with0skipped; local/live/ctest-day2-137.log. The initial command-status poll in7B558 also propagates a
stop before later command mutations.
