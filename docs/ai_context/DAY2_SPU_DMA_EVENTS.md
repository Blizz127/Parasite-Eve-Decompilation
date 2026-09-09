# SPU DMA completion events

Stage127 restores a host completion path needed by original8D610's SPU memory
clear. Original7D15C registers/enables classF0000009, spec20, mode2000 with
no callback. Original7D614 calls DeliverEvent(F0000009,20) when9B434 is zero;
a nonzero9B434 selects the callback instead. pe_spu_dma_event_audit.py pins
the original executable and executes eight IRQ paths through that dispatch
boundary, comparing the SPU control-bit clear and event arguments. BIOS event
storage/consumption remains an explicit host contract, not original BIOS proof.

pe_libetc.c now retains this registration, enablement and delivered state.
A completion may be consumed once by its matching enabled handle. Issuing a
DMA transfer does not signal completion. pe_spu_dma.c accepts a zero-callback
transfer only when this event is enabled; its existing service copies SPU data
before delivering the event. Service still reads the live guest callback slot,
so changing that slot during a transfer routes completion as the original IRQ
does. Unsupported callbacks and unavailable event delivery retain boundaries.
Reset cancels pending DMA and clears registration/delivery state.

The new test checks missing/wrong/disabled registration, three actual1024-byte
transfers, data visibility before/after service, wrong-handle rejection,
single consumption and reset cancellation. B48A's old callback-lifetime test
now expects the already registered event when its live callback becomes zero;
its prior abort expectation represented the missing provider. All eight B48A
tests and the new event test pass normal and ASan/UBSan.

This does not yet implement8D610's loop, WaitEvent's blocking integration,
physical SPU registers or8CF70/8CB54. Stage125's opening failure remains;
Banshee build125 packages are verified but unpublished. Connect and verify the
complete mode-switch graph before reporting the opening fixed or publishing
a playable update. Full regression outcome is recorded in ACTIVE_HANDOFF.md.

Full regression completed:7/8 CTest targets,1319/1320 native groups pass;
the only failure is the known SKIP2 opening assertion. No new regression
was observed. Runtime tests and audit pass; build125 remains unpublished.
