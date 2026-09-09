# CD queued-command recovery

Stage139 translates error recovery7E704 and completion writer7EB88 in
`pc_port/game/boot/cd_stream_port.c`. The data-ready error arm in7F88C now
executes this recovery and propagates any unknown callback stop.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7E704..7E8F4 | 124 | cc34f5b089194c02b861b44bdc80af68287db290a4f3fd5cf00e639d62e56e7e |
| 7EB88..7EC14 | 35 | ca35cee2dd419c7bd2d94030be24ace0c0cb18ff227e1fcec5f7fae3761a5112 |

The recovery scans the eight-slot command queue from its current head. It
publishes completion records when the sequence differs from the immediately
previous queue sequence, initially zero. Saved callbacks are deduplicated
against the last saved callback's sequence. These are distinct rules: a null
callback can change completion deduplication without changing callback
deduplication. Completion records retain the low status byte and eight-byte
response copy; a null response clears only the first response byte. The
completion ring wraps at eight.

All queue indices/counts and the original selected record fields are cleared
before callbacks execute. Record bytes9..11 remain untouched. Known callbacks
use the existing checked dispatcher; unknown identities stop after the
original prefix. A corrupt positive queue count above eight stops before
mutation in native code; the original would overrun its local callback array.
Supported queue and completion indices are0..7.

`python3 pc_port/tools/pe_cd_queue_recovery_oracle.py --check` verifies360
original cancellation graphs/prefixes (328 complete,32 unknown-callback
prefixes) and18 direct completion-writer cases. Cases cover negative/zero/full
counts, both ring endpoints, repeated/zero/alternating sequences, null
responses and callbacks, and status-byte truncation. Known callbacks execute
original813E8->7C564 with MDEC busy; its sector-index increments make callback
counts observable. Native fixtures compare persistent RAM and stopped-call
arguments. An additional native check covers the corrupt-count boundary.

The52 data-ready oracle cases now contain44 complete graphs and8 unresolved
prefixes, because empty-queue error recovery can finish. Normal and
ASan/UBSan focused runs each pass32 DAY2 groups with1297 skipped. Both builds
are warning-free; oracle regeneration/check, Python compilation and scoped
whitespace checks pass. Full CTest passes8/8 in90.21s, including1329/1329
native groups with0skipped; log `local/live/ctest-day2-139.log`.

Automatic CD initialization registration, command-completion callbacks,
physical disc responses/sector FIFO/DMA, MDEC pixels/output/IRQ and movie
player/updater/loader integration remain unfinished. These queue comparisons
do not establish a completed movie or opening-through-Day2 playthrough.
