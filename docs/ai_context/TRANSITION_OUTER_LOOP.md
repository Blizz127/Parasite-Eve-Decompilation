# Original transition outer loop

Authority: original M0000I8019234C..80192740,253 words. SHA256
caa53e35c6e87356c8743445c2bac01cc2b77d97946ea64eb717f077bd3fc0e8. Saved disassembly:
local/live/original-9234C-full.txt (also contains following functions).
The native bootstrap still reaches an explicit8019234C boundary. Do not remove
that boundary merely because the renderer and constructor now have ports.

## Ordered behavior

1. Clear8009CDDC and byte8019C00E. Construct96498, set91DE8(mode!=10),
   seed BIOS rand with VSync(-1), store rand%3000000 at8019C03C, overriding
   with120000 for mode10. Initialize retained empty-run index and effect handle0.
2. If signed half8019C024 is nonzero, skip frames. Otherwise initialize the
   retained effect-start flag0, GPU-address OR mask80000000 and tag maskFFFFFF.
3. Every frame calls3EB04 before checking held mask0F000006. Matching the whole
   mask calls VSync(0), SetDispMask(0), sets exit byteC00E and halfC024 to1,
   calls6A25C, and still calls942FC. Update runs before the exit-byte check.
4. Without the exit byte: F05C, F92C(8019C330), optional94108(signedC02A)
   with halfword result writeback,92740,92800,93478, in that order.
5. First rendered frame starts effect91E30(0xABE,*801EA578+1C), retains its
   result and sets the flag. Later frames call91EFC(handle,*EA578+1C) only when
   long8019C0C0 is zero. A zero handle does not retry the start.
6. Call37870 if mode10, or if byteC044==1 and byteC045==0. Then compact the
  4096-entry ordering table using the loop described below.
7. Store VSync(1) at8019CC14, DrawSync(0), VSync(2),93AB0, ResetGraph(1),
   PutDrawEnv(descriptor+8), PutDispEnv(descriptor+64), DrawOTag(*desc[1]+3FFC).
   Reload descriptor between calls. If current descriptor is8019C1F8 choose
  8019C270, otherwise choose8019C1F8. Load next OT before publishing descriptor,
   toggle8009CDDC with XOR1, ClearOTagR(nextOT,4096), then9BF8C(currentdesc).
8. Repeat while signed halfC024 is zero. At exit call92030 only if byteC00E
   is zero. The button-reset path skips normal transition cleanup.

## Ordering-table compaction

801925A0..8019262C is35 words, implemented by PE_TransitionCompactOT.
It captures the descriptor once, reloads its OT pointer each iteration, and
visits4095 down through0. An empty bucket is exactly `(tag|80000000)==bucket-4`;
this comparison retains the original tag high byte. On entering an empty run,
remember its highest index. On encountering the next nonempty bucket, write its
24-bit address into the remembered entry, bypassing only that empty run. Do not
replace this with a generic relink of every bucket. The remembered index is an
outer saved register retained across frames and returned by the native helper.
The helper is linked and compared in isolation; outer integration remains open.

## Remaining dependencies exposed by this trace

The current3EB04 port is explicitly only a digital-edge cut. Original348-word
3EB04 also calls PadGetState825C0, handles disconnect/reconfiguration via82974,
82680,8292C,828F4, updates32 hold counters at800A7770, checks the special input
sequence, applies additional input-priority masks, and handles analog thresholds
with62A34. Those controller SDK functions are not currently native. Their stack
writes also precede update's borrowed menu bytes. Restore these paths before
claiming a complete outer loop or supplying a fixed menu tail.

HostFB_VSync currently returns void. Original73A44 returns the absolute vblank
counter for negative modes and the entry-time16-bit scanline delta for mode1
and waiting modes. Waiting modes also update the previous-vblank and scanline
baselines. The outer loop uses both query results, so a void shim or returning
its invocation count cannot preserve behavior. This turn documents the gap;
it does not change host timing or claim the timing contract restored.

## Constructor stack evidence

pe_transition_outer_stack_audit.py observes original constructor stores with
entry SP801FEFD0 (the outer frame is30 bytes). Update's menu list consequently
starts801FEF98;942FC later writes only its first halfword. Eight executions vary
mode0/1/2/10 and initial stack bytes00/A5. Under the existing seven constructor
provider contracts, last writers are96F8C for list bytes0..3, no observed writer
for bytes4..7, and96EF8 for bytes8..9. The unwritten halfwords retain0000 orA5A5.
Thus constructor execution alone does not justify zero-filling the menu tail.
Providers may themselves write those stack bytes; input/SDK/previous-frame
history remains necessary. This evidence is not a production menu context.
Output is ignored local/live/transition-outer-stack-audit.json.

Before extending3EB04, audit the storage split for8009D1A0:3E974 currently
updates the host scalar D_8009D1A0 while original3EB04 accesses the guest address.
A full input port must reconcile that existing ownership consistently, rather
than creating another disconnected state copy. Prior digital-edge fixtures also
bypass controller identity; update those fixtures with real pad context when
restoring the complete entry behavior.

DAY1-56 restores825C0/82680/828F4/8292C/82974 and real installed callbacks.
The outer stack audit now also executes twelve complete original3EB04 digital
paths without replacing input/SDK callees. State6 with flagsC000 calls828F4;
83BD0 saves8008291C at the first four menu-list bytes, making the retained second
halfword8008 instead of0000. Busy rejection happens after that stack write and
does not remove it. Other tested controller paths preserve the constructor's
96F8C writer. Bytes4..7 retain initial00/A5 in every case. Remaining constructor
provider and earlier-frame histories prevent binding a final production tail.

DAY1-57 replaces the game3EB04 partial cut with all348 original words and
unifies named/address accesses to D1A0 and D280 in guest RAM. Original/native
full-handler comparisons now cover controller setup, hold counters, masks,
analog and digital edges. The previous storage-split warning is resolved by
single guest aliases, not copies. Timing and remaining stack/serial/live work
above still constrain outer integration.

Actual bootstrap stack (DAY1-57 correction): original80012320..12344 stores the
previous SP at1F8003FC and calls9234C with SP1F8003F8. Its constructor/input call
SP is1F8003C8, so update's menu starts1F800390. The audit's --scratchpad mode
reruns all20 constructor/input cases at that location; writer assertions pass
under the same explicit constructor providers. Earlier1FEF98 observations are
relocated comparison fixtures, not evidence of actual absolute stack values.
Whole-frame provenance must also account for renderer/SDK scratchpad writes.
Evidence: local/live/transition-outer-scratchpad-audit.json.

DAY1-58 observes original VSync at SP1F8003C8 with264 clock-sequence cases.
Negative/mode1 queries do not write menu1F800390..399. Waits leave the watchdog
word via73BC4/73BF0 in bytes0..3 and saved return-address low halfword3B20 via
73BDC in bytes8..9. Bytes4..7 retain the initialA5 fill. Timeout can leave the
watchdogFFFFFFFF, changing the second list halfword to-1. These are original
source dataflow observations under explicit device sequences, not final live
menu-tail values. See VSYNC_CONTRACT.md and local/live/vsync-stack-audit.json.

DAY1-59 restores the original VBlank RNG/four-timer callback and connects CPU
source0 to checked7440C dispatch.512 original comparisons plus four IRQ scenarios
pass normally and under ASan/UBSan; full suite1261 groups, CTest8/8. Unknown or
stopped callbacks prevent subsequent slot execution. Timer1 MMIO, host VBlank
production and VSync adapter remain outstanding; no new live-frame/menu-tail
or full-day acceptance is established. See VSYNC_CONTRACT.md.

DAY1-60 restores ResetCallback's timer1 mode107 setup, with an explicit
HBlank/VBlank edge counter for that original configuration.32 original setup
prefixes and native hardware-contract tests pass; ASan/UBSan targeted pass,
CTest8/8 and1262 native groups pass. GPU edge scheduling, BIOS delivery policy,
clock adapter and full-frame/live validation remain unfinished. Details and
hardware source: VSYNC_CONTRACT.md.

DAY1-61 restores all318 PutDispEnv words and GP1(05..08) display-register
state/readback.2048 original software comparisons and ASan/UBSan checks pass;
full CTest8/8,1263 native groups. This supplies original mode/range inputs for
the remaining GPU scheduler. It does not yet establish scanline timing, public
VSync integration, final menu-stack history or live transition acceptance.
See DISPENV_CONTRACT.md.

DAY1-62 connects explicit GPU blanking edges to timer1/IRQ0 and fixes critical-
section IRQ deferral. It restores BIOS clear-control veneers and controller
reset's formerly skipped VBlank policy write.64 original ABI cases and32 native
edge scenarios pass; ASan/UBSan targeted pass, CTest8/8 and1264 native groups.
Raster signal generation, BIOS handler processing and public VSync integration
remain outstanding. See VSYNC_CONTRACT.md.
