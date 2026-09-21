# CD DMA and physical stream transfer

Stage150 translates the channel3 path of original106-word helper7CEAC and
connects the physical branches of7C564. The reader consumes four location
bytes and discards eight subheader bytes, then transfers the32-byte record
header and2016-byte payload through DMA3. The existing final-record callback
7C214 is now recognized by the shared DMA IRQ dispatcher.

The helper updates channel3's interrupt-enable byte without acknowledging
unrelated DICR flags, enables DPCR, writes MADR/BCR, checks the device request,
and starts CHCR. The CD owner consumes actual FIFO bytes into guest RAM and
latches completion through the existing shared DICR/CPU IRQ bridge. No second
DICR state or direct completion-callback shortcut was added.

The supported DMA controls are11000000 and11400100. Both transfer into RAM;
the latter retains chopped-transfer final-register behavior. Manual transfer
leaves MADR/BCR unchanged; chopped completion advances MADR and clears the
low BCR count. Start/busy flags clear on completion. Zero count means65536
words, so it cannot silently become a zero-length success. Destination and
FIFO bounds are checked before consuming bytes. DPCR-disabled/no-request
transfers remain pending. This follows the register contracts in
[PSX-SPX DMA channels](https://psx-spx.consoledev.net/dmachannels/).

DMA currently completes synchronously when an enabled transfer has its data.
Bus-cycle scheduling, chopping interleaving/arbitration, RAM mirror/wrap cases,
other controls and other channels in7CEAC are not implemented. Busy/request
waits that cannot progress stop explicitly. Disabled-device stream paths keep
their existing unresolved boundaries. Full movie decoding/output remains open.

`pe_cd_dma_issue_oracle.py --check` executes16 complete original idle/request-
ready issuer graphs with MMIO shadow providers. It compares register writes
and interrupt-byte behavior, including low-byte truncation of the sixth
argument. It does not claim equivalence for original timeout paths or DMA
hardware timing. Native integration mounts a disc, issues SDK Setloc/ReadS,
feeds a valid frame sector, then checks every payload byte, location bytes,
record state, FIFO remainder, both control variants, shared IRQ delivery and
7C214's completed-frame publication. Oversized transfer rejection must leave
FIFO bytes and destination RAM intact.

The first integration attempt failed because test reset detached the disc;
the fixture now reattaches it after reset. Full Day1/Day2 and opening-through-
Day2 acceptance are not established. Runtime128 remains published.

## Command-poll data-ready dispatch (shared dispatcher)

`func_8007B010` (B178 loop) and `func_8007B558` (B820 loop) both consume
`func_8007AAB4` acknowledge events and then dispatch `D_8009AFB8` /
`D_8009AFB4`. Those callbacks have native translations (80778, 80164, 7F88C,
7E964, 7F960, 813E8) already reached by `func_8007C13C`, so the two poll loops
now call the same address-keyed dispatcher
`PE_Cd_DispatchDataCallback` (`cd_stream_port.c`, declared in
`platform/pe_sdk.h`) instead of each raising its own
`func_8007B010_afb8_callback` / `func_8007B558_afb4_callback`-style
indirection boundary. Observable effect: a `func_80080D5C` → `func_80080DC4`
blocking command wait that already has a pending data-ready event runs the
real 80778 → 7F88C → 813E8 → `func_8007C564` chain (assembling or dropping
the sector) rather than stopping; `DAY2_movie_player`'s non-video arm asserts
that `movie_retry_wait` is the only stub logged. The default arm of the
dispatcher is unchanged and still stops on an unknown target.

Validation: normal and ASan/UBSan focused runs pass42 DAY2 groups with1297
skipped (1339 total). Full CTest passes8/8 in126.97s, including1339/1339 native
groups with0 skipped. Final builds are warning-free; original oracle/check,
Python compilation and scoped whitespace pass. Logs: `local/live/*day2-150*.log`.
