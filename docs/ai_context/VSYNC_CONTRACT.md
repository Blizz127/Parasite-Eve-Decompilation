# Original transition timing contract

Source:80073A44..80073C54,132 words including the wait helper73BBC.
SHA256 e356692b0a159f0f9e07da321a2ea515c4789379094ff39d6d77f42858169af4.
Native semantic entry: PE_RetailVSync(mode,clock), platform/pe_vsync.c.
PeVSyncClock exposes word reads/writes and the three timeout BIOS operations.
It does not supply fabricated timer values or change the existing host shim.

The EXE data points80094574 to GPUSTAT1F801814 and80094578 to timer1F801110.
Counter800956AC belongs to original7440C callback dispatch;9457C and94580 are
the last completed wait's timer and VBlank baselines. These have different roles
from HostFB_GetState's invocation count and PE_GPU_VSyncQuery's host frame count.

At entry, read GPUSTAT and sample the timer twice until equal. The return delta
is `(timer-previous_timer)&FFFF`, captured before waiting. Negative modes return
956AC; mode1 returns that delta. Both still perform the original entry reads.
For other modes, wait first for previous_vblank+max(mode-1,0), using signed32
comparisons and wrapped32-bit arithmetic, then sample GPUSTAT and wait for
current_vblank+1. When GPUSTAT bit22 is set, also wait for bit31 to change from
the second GPUSTAT sample. Finally publish the VBlank baseline and sample/store
the timer baseline until its immediate readback matches the next timer read.
Return the original entry delta, not the post-wait elapsed time.

73BBC initializes a local watchdog to frames<<15. When below the target it
predecrements the watchdog, stopping onFFFFFFFF. Timeout calls BIOS B(3Fh) puts
with800116FC, B(5Bh) ChangeClearPAD(0), then C(0Ah) ChangeClearRCnt(3,0), in that
order. It then returns to the caller; timeout does not synthesize a successful
counter advance. Native stop epochs only propagate device/provider non-return.

pe_vsync_oracle.py executes all original instructions with explicit device read
sequences and BIOS-service contracts. It records every global/device operation;
no VSync or wait callee is replaced.264 cases cover negative/query/wait modes,
signed counter transitions,16-bit timer wrap, unstable timer samples, interlaced
field polling, and eight stalled-device cases.4846 run-length-encoded records
preserve repeated polling counts. Native execution consumes exactly the original
operation sequence and return value. Private stack traffic is separately audited
with --scratchpad-audit at caller SP1F8003C8.

## Host integration still required

HostFB_VSync remains its previous void provider. It services audio, advances
PE_GPU_VBlankStep by a fixed number for wait modes, and records invocations.
That is not the original relative-wait contract and cannot provide mode1 timing.
Do not equate those existing counters with the original SDK's values.

DAY1-59 connects installed CPU source0 handler7440C to checked callback
dispatch. It increments956AC before scanning eight live slots. The bound3E91C
now advances original RNG and ticks the four timers through36F7C. Unknown
identities or callback non-return stop later slots and retain the IRQ scanner's
active prefix. Existing masking, generation validation and acknowledgement stay
in the CPU IRQ path.512 complete original dispatcher comparisons and four
native IRQ scenarios pass normally and under ASan/UBSan.

DAY1-60 adds the timer1 mode107 lane and restores its ResetCallback setup
write; a general timer MMIO bus and host VBlank IRQ producer remain absent. Calling
callback dispatch directly from VSync would bypass masking and acknowledgement.
Restore timer/source0 timing and BIOS clear policy, then implement a PeVSyncClock
host adapter and bind the public SDK entry. HostFB_VSync still uses its previous
provider; this stage does not establish live timing acceptance.

The outer9234C uses VSync(-1) to seed rand and VSync(1) for8019CC14, followed by
VSync(2). Full-frame menu-stack history and live acceptance remain prerequisites
for claiming the transition integrated.

DAY1-58 observes original VSync at SP1F8003C8 with264 clock-sequence cases.
Negative/mode1 queries do not write menu1F800390..399. Waits leave the watchdog
word via73BC4/73BF0 in bytes0..3 and saved return-address low halfword3B20 via
73BDC in bytes8..9. Bytes4..7 retain the initialA5 fill. Timeout can leave the
watchdogFFFFFFFF, changing the second list halfword to-1. These are original
source dataflow observations under explicit device sequences, not final live
menu-tail values. See VSYNC_CONTRACT.md and local/live/vsync-stack-audit.json.

## DAY1-60: original timer1 initialization and explicit edge counter

Original743B4..7440C is22 words, SHA256
 a8d76ebfddf20244c3904501babf8e4409502f625eefcd21ea740d9fac6c76f2.
At743D0 it writes107 through956B0 (original data1F801114), before clearing
956AC and the eight callback slots and registering source0/7440C. The previous
ResetCallback host implementation omitted this write. It now programs the
platform/pe_timer1.c lane at the same point in the initialization sequence.
The existing canonical installed-pointer adaptation remains; this is not a
new generic MMIO mapper or dirty-pointer implementation.

[Hardware timer reference](https://psx-spx.consoledev.net/timers/) specifies
that mode107 selects HBlank clocks, synchronization mode3, no timer interrupts,
and no target reset. The lane pauses until the first supplied VBlank edge,
then counts supplied HBlank edges with16-bit wrap. Mode reads expose and clear
reached-target/FFFF flags; target remains hardware-reset zero in this lane.
Programming the mode resets count, flags and first-edge gating. Guarded repeated
ResetCallback calls leave the running counter intact. SDK reset clears the
lane and invalidates old IRQ-generation edge tokens. Reads do not advance time.

pe_vblank_init_oracle.py executes32 original initializer prefixes and the real
table-clear callee, stopping at the source0 registration boundary. It relocates
the mode pointer to fixture RAM to observe the software write. This evidence
proves the original setup, not hardware timing. Native tests independently cover
the documented counter/edge contract, stale events, wrapping and reset behavior.

This is specifically the configuration installed by the original VBlank SDK.
General timer modes, cycle-level bus delays, timer IRQs, GPU event scheduling,
the PeVSyncClock host adapter and live timing acceptance remain outstanding.
The old HostFB_VSync shim is still in use. No fixed frame/scanline count has
been substituted for the missing GPU scheduler.

DAY1-61 restores all318 PutDispEnv words and GP1(05..08) display-register
state/readback.2048 original software comparisons and ASan/UBSan checks pass;
full CTest8/8,1263 native groups. This supplies original mode/range inputs for
the remaining GPU scheduler. It does not yet establish scanline timing, public
VSync integration, final menu-stack history or live transition acceptance.
See DISPENV_CONTRACT.md.

## DAY1-62: blanking signal edges and BIOS clear controls

PE_GPU_SetHBlank/SetVBlank accept physical signal levels and an IRQ-generation
token. A rising HBlank ticks the mode107 timer1 lane; a rising VBlank releases
its first-edge gate and asserts I_STAT source0. Repeated levels do not generate
another event. Stale inputs cannot change either levels or device state.
Device reset starts the signal inputs high; the producer must supply the actual
subsequent transitions. Edge counts are diagnostics, separate from the legacy
synthetic GPU VSyncQuery counter and original callback counter956AC.

The edge producer does not dispatch CPU callbacks. The existing scanner now
also defers while the host critical-section depth is nonzero, preserving the
pending status until all nested sections have exited. Masking and generation
checks remain in that scanner. Multiple masked edges coalesce in I_STAT, as a
hardware latch, rather than becoming a queue of callbacks.

Original BIOS veneers73C74 and73C84 (six words excluding padding) have SHA256
88a578c1ecfaeee8cbbae622a3423ee1d4231699ad9ad4d96959f527588a3887.
pe_vblank_bios_oracle.py executes64 original forwarding cases through B(5B) or
C(0A) entry. Native controls preserve full flag values and prior-value return
for counters0..3. Counter values outside that range stop at an explicit boundary.
Controller reset82534/82CF0 now performs its original73C84(3,0) call while in
its critical section, restoring a formerly omitted clear-policy write.

The [BIOS reference](https://psx-spx.consoledev.net/kernelbios/) places automatic
acknowledgement after each BIOS handler's processing. These controls therefore
do not themselves clear I_STAT. The BIOS Pad/Card and timer-handler processing
chain is still unported; the CPU scanner must not be described as that chain.
Likewise the [GPU reference](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/)
distinguishes video-output blanking from timer/interrupt signals. The new signal
inputs do not establish exact raster phase, fractional periods or scanout timing.
A scheduler must derive and supply those signals before live VSync integration.

Native tests cover32 edge/mask/nested-lock scenarios, coalescing, timer gating,
stale-input state retention, BIOS controls and controller-reset policy. This is
hardware/provider integration evidence, not an original whole-frame comparison.
