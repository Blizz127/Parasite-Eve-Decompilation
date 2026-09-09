# CD command-completion state machines

Stage140 translates the registered80164 callback and its80220,80404 and8068C
branches in `pc_port/game/boot/cd_stream_port.c`. CPU CD interrupt dispatch
now recognizes80164. The callback updates response status through8080C,
clears pending flags, forces status5 on the device error bit, routes by command
kind, and sets kind33 only after a completed branch leaves no pending command.
Unknown nested callbacks stop before this final cleanup.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 80164..80220 | 47 | 3f914f15268342c1e5813fe55c7d56b5a419572b9d97b1e956fc0995a6f73d40 |
| 80220..80404 | 121 | bac5290de82e938bb8034314056c4e9780ad94fab23e8224e2d4ad252fb94546 |
| 80404..8068C | 162 | c1e84ce0b64fd483bcd85563e24d4cff81d138cfaefafdce980fbf26de32fa9f |
| 8068C..80778 | 59 | 816d364134d9cfef47726315e066cce03cabc424bd35a9b30b28b151447619b9 |

Public-command completion preserves the original live jump table at11D0C,
SetMode bit80 transitions, unaligned four-byte location copy, command-byte
publication, read/play timeout initialization and callback-enable gates.
Unknown live table targets retain explicit boundaries. Internal completion
handles states12..17, startup phases21..24, the signed timer threshold301,
exact status-byte comparisons and data-before-command callback ordering.
Startup notifications throughA36AC are independent of theB554 enable gate.
The remaining-completion branch retains its narrower SetMode/error behavior.

`python3 pc_port/tools/pe_cd_command_oracle.py --check` authenticates the
executable and checks3314 original full callback graphs/prefixes:2716 complete
and598 at unknown nested callbacks. Public/default cases vary all32 command
bytes, status truncation, device flags, mode bit80, callback presence and enable
gates. Internal cases vary state, phase, timer threshold, flags and callbacks;
additional cases cover a nonzero read/play delay. Native tests compare the
complete80-byte SDK status region and exact stopped-call target/arguments.
The original registered entry executes8080C and all three state machines;
no command-completion state changes are supplied by a provider.

The52 interrupt/data-ready oracle cases now have48 complete graphs and4
unknown-data-callback prefixes. Their command-completion cases execute80164
through the existing native CPU source2 integration test.

Stage141 translates7E964 and7E5C4; see
[queued completion](DAY2_CD_QUEUE_COMPLETION.md). The stage140 source inspection
saved at `local/live/cd-queue-completion-140.asm` shows publication, retry
counts, queue advancement/removal, a global callback and pending-command
restart through7FBF0/7FB44. Initialization7BBFC still needs its correct reset
and registration sequence.7FBF0 and7FB44 already have native implementations.
The translated7E5C4 original span7E5C4..7E6B0
removes consecutive matching sequence records from the head, preserves record
bytes9..11, decrements count and resets the current index to the resulting
head. Source is saved at `local/live/cd-queue-remove-140.asm`.
Physical disc responses/sector FIFO/DMA, MDEC
pixels/output/IRQ, movie player/updater/loader and the full opening-through-Day2
playthrough remain unfinished. These comparisons do not prove either day complete.

Normal and ASan/UBSan focused runs each pass33 DAY2 groups with1297 skipped.
Both final builds are warning-free; original fixture regeneration/check,
Python compilation and scoped whitespace pass. The first comparison caught
an incorrect phase22->24 transition; the final port preserves22->23->24.
Full CTest passes8/8 in87.54s, including1330/1330 native groups with0 skipped;
log `local/live/ctest-day2-140.log`.
