# CD queued completion and retry

Stage141 translates7E964 and7E5C4 in `pc_port/game/boot/cd_stream_port.c`
and adds7E964 to the checked CD callback dispatcher. This connects the SDK
command-completion handler to queue completion, retries and removal.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7E5C4..7E6B0 | 59 | e7239bb1c9421072ce01869eae99b016c60da5f87bdf6fbd1c2a1fbf5c3a47c6 |
| 7E964..7EB88 | 137 | 70a363e79ed71476b8961d2cb1c2a537c881a4c232bd3902a148f31bef32fc11 |

7E964 publishes the current nonzero sequence/status/response mailbox. Status2
advances the current index if the next record has the same sequence; otherwise
it publishes a completion, calls the record callback and removes the head's
consecutive sequence group. Status5 resets the current index to the head when
retries are positive or minus one. Positive retries decrement; minus one means
unlimited retries. Other retry values complete with status5. Unknown callbacks
stop before the original remaining mutations.

7E5C4 preserves record bytes9..11 while clearing the original selected fields,
advances/wraps the head, decrements count and finally copies head to current.
The current record callback runs before removal, and the globalB8AB0 callback
afterward. All completion paths can reach the global callback. A ready drive
with a positive queue count and nonzero current record calls the existing
7FB44 command issue path after its second readiness read. Callee stops propagate.

`python3 pc_port/tools/pe_cd_queue_completion_oracle.py --check` verifies5184
original completion graphs/prefixes (3200 complete,1984 unknown callback
prefixes) and30 original removal cases. Variants cover wrapped heads/current
indices, empty/nonempty queues, zero/repeated/changing sequences, truncated
status, negative/zero/positive/unlimited retries, null responses, record/global
callbacks and ready/busy lanes. Known callbacks execute original813E8->7C564
with MDEC busy, making callback counts observable in the sector index.
Original7FBF0 and7FB44 execute; fixtures hold the command-pending gate positive
so these cases verify restart eligibility without issuing physical commands.
Native tests compare the queue, completion ring, SDK state and callback effects,
plus stopped-call target and arguments. Supported queue indices are0..7 and
counts are at most8; negative removal counts follow the original signed gate.

Initialization7BBFC remains to be restored in original reset/registration
order. Its source is saved at `local/live/cd-init-141.asm`: diagnostics,
callback reset, source2 registration, pending-tag clearing, Nop/Init/Demute
commands, then final command status2 check. This work does not add physical
command responses, sector FIFO/DMA, MDEC pixels/output/IRQ or movie
player/updater/loader integration. Opening-through-Day2 acceptance remains open.

Normal and ASan/UBSan focused validation each pass34 DAY2 groups with1297
skipped. Builds are warning-free. Original fixture regeneration/check, Python
compilation and scoped whitespace pass.
Full CTest passes8/8 in120.79s, including1331/1331 native groups with0 skipped;
log `local/live/ctest-day2-141.log`.
