# Mounted-disc sector delivery

Stage149 adds bounds-checked raw-sector access to the existing disc owner and
connects Setloc, SeekL/SeekP, ReadN/ReadS and Pause to the explicitly enabled
CD command device. The byte data port now consumes distinct bytes from the
mounted image. Data reaches SDK notification RAM through the existing source2
interrupt dispatcher, acknowledgment worker and data callback.

The protocol follows [PSX-SPX CDROM](https://psx-spx.consoledev.net/cdromdrive/):
Setloc accepts three packed-BCD fields; reads acknowledge with INT3 and announce
sectors with INT1; Pause acknowledges then completes with INT2. Data request
BFRD exposes 2048 bytes at raw offset24 or 2340 bytes at offset12, depending on
mode bit5. DRQ clears after the FIFO drains. Single/double-speed scheduling uses
75/150 sectors per second. Invalid BCD returns INT5 with parameter error10.

Timing remains approximate. Each service call publishes at most one response;
command acknowledgments serialize with sector responses. Seek/Pause completion
uses the existing deterministic100000-cycle delay. Mechanical seek latency,
error correction/retries, multi-sector hardware queues, pregap/end-of-disc
behavior, CDDA/XA decoding and mode bit4 behavior are not implemented. Unsupported
read modes, out-of-image locations/reads and FIFO underflow stop explicitly.
Unread pending-sector cases hold (backpressure) until BFRD clears the sector —
`HostFB_PumpCdProgress` can outrun INT1→BFRD→DMA3 on live FMV; STOP-on-overrun
was retired as an artificial wall (DAY2-158v). DAY2-158w folds dig/cd-sector-overrun: while B0CD0 owns sector_pending, HostFB_PumpCdProgress stalls DeviceTime and still services IRQs so 91DC8 can BFRD (158v hold-only hung live). The model retains one pending
sector plus an independent requested FIFO. It does not invent multi-sector
hardware queues or claim hardware overrun-bit fidelity.

`test_cd_sector_device.h` checks raw bounds and every byte of consecutive
sectors in both size/speed modes, delayed arrival, BFRD/DRQ, separate seek/pause
responses, invalid BCD and pending-sector backpressure (hold + catch-up after
BFRD). A separate integration
sequence calls public initialization and SDK Setloc/ReadS, advances host VSync
queries, checks SDK data-ready publication and consumes the mounted sector.
No test provides command returns or writes SDK completion fields. The original
table extractor now also pins26 command-completion jump targets. These are
protocol/integration tests, not a new original-code control-flow oracle.

DMA3 and the stream reader's physical header/body transfer remain disconnected.
Next connect those transfers, then MDEC output and movie player/updater/loader.
Default device activation and complete opening-through-Day2 acceptance remain
outstanding; Day1 is not yet verified100% complete. Runtime128 stays published.

Validation: normal and ASan/UBSan focused suites each pass41 DAY2 groups
(1297 skipped,1338 total). Full CTest passes8/8 in135.89s, including1338/1338
native groups with0 skipped. Final builds are warning-free; authenticated
table regeneration/check, Python compilation and scoped whitespace pass.
Logs: `local/live/*day2-149*.log`.
