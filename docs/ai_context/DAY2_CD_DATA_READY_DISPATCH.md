# CD data-ready dispatch into stream assembly

Stage138 extends cd_stream_port.c with the original data-ready callback
chain and adds its7C13C identity to the existing CPU IRQ dispatcher. A
registered source2 interrupt can now acknowledge a device response, update
SDK response state, call the stream callback and assemble a RAM-backed sector.
This is not live disc delivery or decoded movie playback.

Original executable SHA1 is452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7C13C..7C214 | 54 | 475233be54105c3ebe9365cd3945f58a8a54514242cdedd5223b46132f1d8d2b |
| 80778..8080C | 37 | 397415addab887871dedae54e213e4c08e24c963ba204c5c2ddcbc7c80fe0ec4 |
| 8080C..808BC | 44 | 9de3706b3f6f6f7305512148b71d4c3492217e754c316b39eeceb5f571d3f880 |
| 80998..809E0 | 18 | db8701abfa4819e946cb3f39a9e93491e64bbdaa2b63fe1f6cb954b8ac791330 |
| 7F88C..7F960 | 53 | 339f9403a21833ba2629437aaabecca5ddeb3dc6e0fe1147d8faaec923164b42 |
| 813E8..81408 | 8 | 0a54e2df91311e97e5ae88a9dc0b0ca485386c20ae1bb2a109b83ae193f7edd0 |

7C13C saves the low CD bank bits, repeatedly calls7AAB4 and dispatches data
callbacks before command callbacks when both result bits are set. Callbacks
receive the original pending status byte and response address. Bank restoration
occurs only after the worker returns zero. Unknown targets and callee stops
propagate before this cleanup.80778 updates response-derived device status
through8080C, handles status bit10 and applies both callback-enable gates.
8080C uses the command's response-length table to select the status byte,
unless incoming status5 selects byte0; a negative selected offset exits.
It publishes four status bits and the original eight-byte response copy.

7F88C publishes the appropriate data/report mailbox, then dispatchesB8AB4.
Stage139 translates its status5/bit10 recovery call7E704; see
[queue recovery evidence](DAY2_CD_QUEUE_RECOVERY.md).813E8 is the
original argument-ignoring wrapper into7C564. The18-word80998 response-copy
semantics are represented byCdCopyResponse: forward byte copy, null-destination
no-op, and null-source clearing only the first destination byte. Command
callback80164 is translated in stage140; see
[command completion](DAY2_CD_COMMAND_COMPLETION.md). Other unknown callback
identities remain boundaries.

`pe_cd_dispatch_oracle.py` verifies the executable and executes52 complete
original callback graphs or prefixes:48 complete and4 at unresolved calls
after stage140 (stage139 had44 complete and8 prefixes).
It starts at7C13C with interrupt-active state set. The CD byte-read/ack
providers from stage137 and VSync provider supply hardware-facing values;
all SDK/data/stream callbacks and RAM sector copies execute original code.
Response and sector payload bytes are distinguishable. Variants cover
complete/partial stream records, active guard, disabled forwarding, null
callbacks, bad sector headers, status4, error recovery, unknown command/data
callbacks, zero response-length entries and status bytes selected at an offset.
Two payload patterns and two original CD banks are compared.

The native test enters throughPE_IRQ_ServicePendingForGeneration with a
registered7C13C callback and source2 asserted, using the actual response
FIFO. It compares original persistent RAM including20KiB of record/payload
memory, and checks CPU source acknowledgment, interrupt-active cleanup,
normal CD bank restoration, and exact unknown-callback argument contracts.
The original oracle does not execute the CPU exception wrapper; its native
integration is an additional check of the previously translated IRQ service.

The retained stream-location local is explicitly seeded at original1FEF68
and native801FFE88 for this call depth. General stack residue, nested IRQ
reentrancy and device timing remain unverified. Registration and interrupt
assertion are arranged by the test: automatic CD initialization registration,
disc-command response production and interrupt generation are not added here.
Physical sector FIFO/DMA and MDEC decode/output remain unfinished, as do
movie player/updater/loader integration and opening-through-Day2 acceptance.

Normal and ASan/UBSan focused validation each pass31 DAY2 groups with
1297skipped; builds are warning-free. Original fixture regeneration/check,
Python compilation and scoped whitespace pass. Full CTest passes8/8 in
87.78s, including1328/1328 native groups with0skipped; log
local/live/ctest-day2-138.log.
