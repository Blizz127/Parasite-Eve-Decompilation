# Libpress input, DMA1 output and movie slice delivery

Stage152 connects the software decoder to original libpress calls and the
existing movie slice callback. BFA0 preserves the original command-bit edits
and forwards its low-halfword length to input submission. C01C/C27C waits for
prior output, enables DPCR, rounds the request down to32-word blocks, then
programs DMA1. Zero encoded block count retains hardware count semantics;
it does not become a completed zero-byte transfer.

Submission does not deliver decoded pixels or invoke a callback. Host VSync
services MDEC DMA: input completion copies the latched command's run-length
payload into decoder-owned storage; output completion drains pixels into RAM,
updates MADR/BCR/CHCR and latches the shared DMA completion flag. Data-request
control and DPCR enable bits gate service. The shared CPU dispatcher recognizes
1214D4, whose continuing-slice arm queues the next DMA1 request before uploading
the previous buffer. Final slices retain their original frame flag/bank updates.
Output calls without any supplied decode retain the diagnostic callback stop.

Input-table telemetry remains available; table DMA now also completes when
serviced. Its actual table contents are retained by the MDEC owner. Input
submissions wait for the previous DMA and stop if its gate prevents progress.
Requests that cannot decode or fit in RAM stop without delivering a completion
IRQ. The original C1EC also waits on MDEC command-busy through C308; that
status/wait path remains to be connected for overlapping commands. Timing is one host service checkpoint, not measured MDEC cycles. Command
pipelining, hardware FIFO pressure and hardware-exact numeric rounding remain
unverified; this change does not upgrade the pixel-fidelity claim in
[DAY2_MDEC_PIXELS.md](DAY2_MDEC_PIXELS.md).

`pe_mdec_io_oracle.py --check` authenticates the original overlay and checks24
input-wrapper graphs and16 output-issuer graphs. Input transfer and the idle
output wait are explicit original-execution providers; output register writes
are compared separately from device completion. Cases include high mode bits,
request truncation and low address bits.

The native integration uses the game's authenticated tables and a two-
macroblock run-length command. It verifies that submission preserves the old
output buffer, DPCR masking stalls DMA1, CPU IRQ masking permits the data copy
but defers the callback, and unmasking reaches the real1214D4 callback. The
callback queues the second slice; final GPU completion produces two distinct
gray tiles at the expected VRAM coordinates in15-bit and24-bit modes. Adjacent
pixels remain untouched. No command-return, decoded-pixel or callback-return
provider is used in that integration.

Initial VRAM validation failed with correct decoded RAM buffers. The fixture
had enabled GPU DMA before the first ResetCallback, which resets DPCR. Moving
callback initialization before GPU setup fixed the fixture's ordering; no
expected pixel values were changed.

The first full regression run found one obsolete real-disc assertion that
expected the last table DMA to stay active. It now checks completion count2,
advanced MADR and cleared busy/block count after host polling.

Next implement the remaining libpress waits/status and movie player121C04,
updater122040, loader14E30 and bitstream-to-run-length calls. The older movie
overlay also registers a distinct80191DC8 callback that remains unported. Exercise the full
player with physical stream callbacks and real movie frames. Runtime128 stays
published; opening-through-Day2 and100%Day1 acceptance remain unfinished.

Final validation: normal and ASan/UBSan focused runs each pass44 DAY2 groups
with1297 skipped (1341 total); the corrected real-disc case also passes in
both builds. Full CTest passes8/8 in120.88s, including1341/1341 native groups
with0 skipped. Final builds are warning-free; original oracle/check, Python
compilation and scoped whitespace pass. Log: `local/live/ctest-day2-152-final.log`.
