# Connected low-level CD initializer

Stage146 translates7F994 in `pc_port/game/boot/cd_stream_port.c`. It calls the
translated controller initializer, audio setup, SDK state initializer and mode
setter in original order, installs command/data callback identities, binds the
native7FE24 body, installs VBlank slot0 through73D58, and publishesAFD8/B554=1.
The host binding has no guest-state counterpart; it resolves the original
installed guest identity to its translated native body.

The public7EC14 already-initialized path now returns0 as the original EC2C
delay slot requires. Its prior return ofB554 was incorrect. Its cold path
still uses the earlier collapsed host initialization; replacing that path
requires successful device command-response integration, not just helper ports.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7F994..7FA2C | 38 | 2dbb8f474e165989f8195bab136705ba966bd7fca2e0b69738c1d0dd9e7452d6 |
| 7EC14..7ED58 | 81 | 26f2de2fd362c72fda26ab8368fb82d03a4456e898d42a65d0c43764b608b7f1 |

`pe_cd_lowlevel_init_oracle.py --check` executes32 complete original low-level
initialization graphs, half followed by actual original7440C VBlank dispatch,
and8 original public guard graphs. Original subroutines execute, including
registration, audio, mode, callback setters, SDK setup and the updater. Console
calls are providers with asserted original arguments; any VSync query uses
the matching counter. Native tests compare guest callbacks, SDK fields, mask,
SPU/CD registers and installed updater effects. Eight public guard cases verify
zero return and unchanged state for several nonzero flag values.

As in stage143, these fixtures require missing parameters for Nop/Init to
exercise ordinary rejection without synthetic completion events. Original7F994
ignores7BBFC's ordinary failure return and continues; native preserves this.
An unresolved native boundary still unwinds before subsequent publication.
Thus a completed fixture proves this original failure-path behavior and helper
integration, not a working drive or successful cold startup. The first tick
exercises an attempted internal command through the real issuer and its
rejection gate. Successful command response delivery remains unimplemented.

Next is device response production and public cold-path wiring, followed by
physical sector FIFO/DMA, MDEC pixels/output/IRQ and movie player/updater/loader
integration. Full opening-through-Day2 acceptance remains unfinished.128 stays published.

Normal and ASan/UBSan focused runs each pass39 DAY2 groups with1297 skipped.
Both builds are warning-free; fixture regeneration/check, Python compilation
and scoped whitespace pass.
Full CTest passes8/8 in119.54s, including1336/1336 native groups with0 skipped;
log `local/live/ctest-day2-146.log`.
