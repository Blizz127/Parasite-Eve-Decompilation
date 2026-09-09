# Public CD startup and host VBlank delivery

Stage148 connects public initializer `8007EC14` to the translated low-level
initializer when the mounted-disc command device is explicitly enabled.
Its already-initialized guard remains zero-returning. The existing collapsed
cold path remains for callers that have not enabled the command device.
This is not automatic activation in the published runtime.

The original `8007F960..8007F98C` notification wrapper is now translated and
recognized by the CD callback dispatcher. It calls the optional pointer at
`800B8AB8` with the low status byte and unchanged response pointer. Unknown
callback targets retain a diagnostic stop. The authenticated original-code
oracle `pe_cd_startup_notify_oracle.py --check` covers 20 cases: 10 complete
null-callback returns and 10 prefixes ending at an unknown callback entry.
Native tests compare the callback target and both arguments.

For enabled devices, HostFB_VSync advances the device and delivers pending
interrupts through the existing CPU scanner, excluding recursive SDK IRQ
service. Waiting modes advance 564480 approximate cycles per requested frame;
queries advance 1024 cycles. Accumulated frames update the GPU counter and
produce a VBlank edge through PE_GPU_SetVBlank, which asserts source0.
The CPU dispatcher then reaches the installed SDK VBlank callback. This
clock is an NTSC 60 Hz approximation, not an instruction-timed CPU or full
scanline/PAL timing model. Disabled-device behavior is unchanged.

The mounted-disc integration test now calls the public initializer. It checks
initial commands Nop/Init/Demute and response tags 3/3/2/3, public handler
installation, and VBlank mask behavior: masked source0 leaves the callback
counter and command count unchanged while retaining the pending IRQ. After
unmasking, ordinary host VSync waits drive Nop/Nop/GetTN/Nop and reach lane1,
state11. The test does not manually dispatch startup callbacks or plant SDK
completion fields. Existing transport FIFO, response ordering, parameter and
unsupported-command checks remain in the same group.

Physical seek/read commands, sector FIFO/DMA, MDEC pixels and the full movie
player/updater/loader still need implementation and verification. Opening
through the end of Day2 and 100% Day1 completion remain unproven. Runtime128
remains published.

Normal and ASan/UBSan focused suites each pass40 DAY2 groups with1297 skipped
(1337 total). Both final builds are warning-free. Original oracle regeneration/
check and Python compilation pass. Full CTest passes8/8 in167.77s, including
1337/1337 native groups with0 skipped. Log: `local/live/ctest-day2-148.log`.
