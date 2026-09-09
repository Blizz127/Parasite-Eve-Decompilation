# SPU mode switch and memory clear

Stage128 translates8CB54,8CF70,85A64,85BB4 and8D610's mode-change behavior.
It connects valid music commands to the mode switch instead of stopping on
any mode mismatch. The mode table selects reverb registers, depth globals,
SPU start address and a memory-clear span. Existing event-backed DMA performs
each transfer; completion is consumed only after the service copies data.
Saved callback/transfer state is restored after clearing. Mode4's final DMA
block rounds to64 bytes and wraps through the end of512KiB SPU memory, as
represented by the existing DMA provider.

The register provider stores the512-byte SPU register window and supports
physical1F801C00 plus its kernel aliases. The mode writer also retains its
explicit RAM-register fixture path. DMA completion clears control bits30
before event/callback dispatch. This records register values; it does not
implement audible synthesis or all SPU hardware side effects.

pe_spu_mode_oracle.py runs original8CB54 and its CPU callees, intercepting
7D778's transfer operations and8D7B0's WaitEvent with explicit DMA/event
contracts. Register state uses an explicit RAM window. SPU memory begins
withA5 bytes, allowing the comparison to prove cleared and untouched regions.
Forty original cases cover ten modes, already-configured/new mode and saved
callback/transfer variants. Native tests repeat those cases for RAM and host
physical-register paths, comparing guest RAM, complete SPU RAM and transfer
counts. Provider boundaries are explicit; this is not a full original BIOS
or physical-hardware execution claim.

The full mode graph's results and opening regression outcome are recorded in
ACTIVE_HANDOFF.md. Build125 packages predate this change and remain unpublished.
The full Day1/Day2 and requested Banshee publication work remains active.

Validation for release 128: focused normal/ASan comparisons and opening pass;
full normal CTest 8/8 passes. Linux and Windows Release startup reach the
120-frame limit (Windows under Wine). Both archive inventories, binary/disc
hashes and packaged Linux startup pass. Publication evidence is recorded in
ACTIVE_HANDOFF.md and local/live/publish-day2-128.
