# Device-driven CD startup responses

Stage147 adds an explicitly enabled command device to `pe_cdreg.c`. Enabling
requires an active validated disc and an explicit interrupt mask; it models
the post-BIOS mounted, motor-on, single-track state. Reset disables it. Existing
manual response ingress/shadow tests remain available while the device is off.
Stage148 connects the public cold initializer when the device has already been
explicitly enabled; normal runtime does not automatically enable it. See
[DAY2_CD_PUBLIC_STARTUP.md](DAY2_CD_PUBLIC_STARTUP.md).

Supported commands are Nop, Init, Mute, Demute, SetMode, Getparam and GetTN.
Bank0 writes capture parameters and submit commands. A bounded device clock
publishes response bytes/tags through the response FIFO and asserts CPU source2
when the device interrupt mask permits. HostFB_VSync services enabled devices
and invokes the existing CPU scanner when an enabled interrupt is pending and no SDK interrupt
is active. SDK state is changed only by translated acknowledgment/callbacks;
the device never writes SDK completion fields.

Init produces INT3 followed by INT2. An unacknowledged response prevents the
next response from overwriting it. Incorrect parameter counts produce INT5
with error20. Unsupported commands, parameter overflow, overlapping unsupported
commands and active-media changes remain explicit native boundaries. Repeated
Init while Init is pending is dropped. Volume-register behavior remains separate.

Protocol evidence is the original command-table/callback code and the hardware
research documentation [PSX-SPX CDROM](https://psx-spx.consoledev.net/cdromdrive/).
It specifies Init's two replies, the interrupt-mask AND rule, FIFO acknowledgement,
command results and parameter errors. The default response delay uses measured
PSone valuesC4E1 (general) and13CCE (Init first response). Init completion delay
100000 and the host1024-cycle service quantum are deterministic approximations,
not verified retail timings. Stage148 uses564480 cycles per waiting-mode frame
and generates VBlank edges; a complete CPU/device clock model is still required
for timing fidelity.

`pe_cd_device_tables.py --check` authenticates the executable and pins six
original32-word SDK tables (192 words). This is fixture extraction, not a new
original-control-flow oracle. The native integration test mounts a validated
in-memory disc, enables the device, and calls7F994 with unmodified original
command tables and first-time ResetCallback. It receives commands1/A/C and
tags3/3/2/3 through source2, then dispatches the installed VBlank updater four
times. Nop/Nop/GetTN/Nop completion reaches SDK lane1/state11. No test plants
completion statuses or supplies command return values.

Transport checks cover delayed first response, device-mask gating, deferred
Init completion, mask re-enabling, SetMode/Getparam FIFO round-trip, parameter
errors and an explicit unported ReadS boundary. The test confirms a working
supported startup path, not physical seek/read timing or sector delivery.

Stage148 completes the explicitly enabled public cold path and7F960 wrapper,
and replaces manual startup ticks in this test with host VSync waits. Next: remaining
seek/read commands, sector FIFO/DMA, MDEC output and movie player/updater/loader.
Full opening-through-Day2 acceptance remains unfinished.128 stays published.

Normal and ASan/UBSan focused runs each pass40 DAY2 groups with1297 skipped.
Final builds are warning-free; original table regeneration/check, Python
compilation and scoped whitespace pass.
Full CTest passes8/8 in122.93s, including1337/1337 native groups with0 skipped;
log `local/live/ctest-day2-147.log`.
