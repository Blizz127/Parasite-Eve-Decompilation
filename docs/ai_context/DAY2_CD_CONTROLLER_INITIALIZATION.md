# CD controller initialization and SDK startup state

Stage143 translates7BBFC and7FA2C in `pc_port/game/boot/cd_stream_port.c`.
They are available as native entrypoints. The collapsed outer7EC14/7F994
startup path is still unchanged: wiring it requires the remaining device
response production, audio setup and callback installation dependencies.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7BBFC..7BDDC | 120 | 999df4d77574d9aeefc84bb49b2d82849c3daab18f43373e2e1c4974cc35c113 |
| 7FA2C..7FB04 | 54 | 738251284efcede9226741ec4573c44ad6323ca8ee892b71313abac82f501f80 |

7BBFC clears callback/status fields, calls ResetCallback, registers source2,
selects CD bank1 and acknowledges pending tags until empty. It clears the
pending data statuses, sets command status2, selects bank0, clears the request
byte and writes1325 to the mailbox. It issues Nop, optionally repeats Nop on
the status bit10 condition, issues Init and Demute with failure returns, then
requires final status2. Calls use the existing issuer/poller; any unresolved
callee stop propagates. BIOS startup diagnostics are adapted to a fixed host
log line with the original address argument; BIOS console formatting and
mutable guest diagnostic text are not reproduced.

7FA2C reproduces the original selective byte/word clearing and initial lane2,
state14, phase21, and counter1. Its original80B44(0,B582) call is represented
by the resulting00:02:00 BCD location. Location byte3 and other untouched bytes
remain intact. This specialization applies only to this fixed LBA0 call.

`python3 pc_port/tools/pe_cd_initialization_oracle.py --check` compares four
complete original SDK state graphs with different nonzero padding and36
complete rejected-command initializer graphs. The latter execute original
ResetCallback's initialized-guard path, installed73CC4/740D0 registration and
7B558's missing-parameter failure path. Only the two console calls are
providers; their original arguments are asserted. I_MASK/CD MMIO are redirected
to RAM in the original interpreter, and mapped to native device owners in the
native test. Final guest state, device registers, mask and failure return match.

These startup tests deliberately set Nop/Init parameter-table entries nonzero
to exercise rejection. They do not represent a successful retail startup.
First-time ResetCallback integration, pending-tag draining, repeated Nop,
successful Init/Demute responses and final success remain unverified here.
The full control flow is translated, but those paths need device-driven checks.
The existing no-response command wait/diagnostic boundary is retained.

Next dependencies in7F994 are7BAC0 audio/register setup,812F4 mode store and
73D58(0,7FE24) VBlank callback installation.7BBFC also needs real command
response production before the outer initializer can be wired and exercised.
Physical sector FIFO/DMA, MDEC pixels/output/IRQ and movie player/updater/loader
remain unfinished, as does full opening-through-Day2 acceptance.128 stays published.

Normal and ASan/UBSan focused runs each pass36 DAY2 groups with1297 skipped.
Both builds are warning-free; fixture regeneration/check, Python compilation
and scoped whitespace pass.
Full CTest passes8/8 in125.01s, including1333/1333 native groups with0 skipped;
log `local/live/ctest-day2-143.log`.
