# Phase 6E-B53I-B2 — DMA2 completion and IRQ delivery

## Result and hard boundary

B53I-B2 connects one explicitly admitted GPU DMA2 completion to the two
retail interrupt-dispatch layers:

```text
captured DMA2 token
  -> hardware data completion
  -> physical DICR bit-31 rising edge
  -> CPU I_STAT source 3
  -> func_80073F00-equivalent CPU scan
  -> func_80074520 DMA scan
  -> DMA channel-2 identity 0x80076EE4
  -> func_80076EE4_idle_pump boundary
```

The idle-DMA queue-consumer body is deliberately not translated. At the
focused boundary, the first image is complete and both interrupt levels have
performed their retail acknowledgement, but the queued second request is
still byte-identical, producer/consumer remain `1/0`, and no second DMA has
been issued.

Starting B53I-B1 commit:

```text
76d8cffc48b4a0c38627ff5dfbb771a704880f33
```

Its parent is `31da8f843241910a70d49cee6e738b4c19bb76fd`.
Before work, the unrelated untracked B53I-B1 runtime-packaging helper was
preserved outside the checkout at
`/home/blizz/dev/parasite-eve-scratch/b53i-b1-packaging/package_runtime.sh`.
Its SHA-256 remained
`631c53097c79e8ae1c557e4d06f4754a5599fade4a381782c0d5c257f4ba8886`;
it is not B2 source and is not part of the B2 commit.

## Retail provenance

All literal recovery was reverified against the exact executable:

```text
SHA-1  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

| Function | Exact range | File offset | Size / words | Body SHA-256 | Semantic ABI |
| --- | --- | --- | --- | --- | --- |
| `func_80073F00` | `0x80073F00..0x800740CF`, end `0x800740D0` | `0x64700` | `0x1D0` / 116 | `4862b3fbc3bb78e49db65b4ebacc3c188f4238d6f4cc9cbd8d2ba3dce59bcace` | `void func_80073F00(void)` |
| `func_80074520` | `0x80074520..0x8007469F`, end `0x800746A0` | `0x64D20` | `0x180` / 96 | `3dd9a2f9f85757a6e5c28f1fa6f48cf929370c6ad4da3ae8c08e08dc852259cd` | `void func_80074520(void)` |

The standalone B2 oracle embeds every word of `func_80074520`, the 50-word
CPU scan/acknowledgement core, and the three-word normal terminal window. It
also verifies the full 116-word CPU body hash. No production C is imported.

## Single DICR authority and physical bit 31

`platform/pe_gpu.[ch]` remains the only native DICR authority. Stored DICR
contains lower/control bits and completion flags 24 through 30, but never the
read-only physical/master flag bit 31. `PE_GPU_ReadDICR()` synthesizes bit 31
from stored state; `PE_GPU_ReadStoredDICR()` exposes raw state for tests and
diagnostics only.

The recovered formula is:

```text
physical_bit31 = force_bit15
              || (master_bit23 && any(stored_flags24_30))
```

Per-channel enables are not part of that derived formula after a flag has
latched. They gate creation of a normal completion flag instead.

Every stored-DICR mutation goes through one transition helper. It compares
the old and new derived levels and sticky-latches a one-shot event on only a
false-to-true transition. A later falling transition cannot erase an
unconsumed rise. The separate bridge consumes that event. Reset clears raw
DICR, the physical level, and any unconsumed edge.

This covers transitions caused by:

- DMA completion;
- DICR software writes, including W1C acknowledgement;
- master/channel-enable changes;
- force-bit changes;
- explicit represented completion flags for channels 0 through 6.

Repeated writes while the physical level stays high do not retrigger source
3. If the level rises and then falls before the bridge, the already-created
edge is still delivered exactly once.

## Revised normal-completion contract

For a valid active DMA2 token, `PE_GPU_ServiceDMA2Completion(token)` always
performs the data side of the hardware event:

1. validate active transfer and token;
2. read/copy the remaining guest source;
3. make every transferred pixel visible in VRAM;
4. publish data order;
5. clear DMA2 active and CHCR bit 24;
6. create DICR flag 26 only when channel-2 enable bit 18 and master bit 23
   were both set at completion;
7. publish completion order and event count.

It does not assert I_STAT and does not run any callback. This deliberately
supersedes B53B's earlier bounded unconditional-flag expectation. A disabled
channel or disabled master still completes data and clears CHCR, but creates
no flag, physical bit-31 edge, or source-3 assertion.

Canonical first completion is:

```text
CHCR stored       01000201 -> 00000201
DICR stored       00840000 -> 04840000
DICR physical     00840000 -> 84840000
edge pending      0 -> 1
I_STAT            remains 0000 until the bridge
```

## Three separately testable phases

The production checkpoint composes three public phases without collapsing
their state:

1. `PE_GPU_ServiceDMA2Completion(captured_token)` changes only the GPU/DMA
   authority.
2. `PE_IRQ_BridgeDICRRisingEdge(captured_irq_generation)` consumes at most
   one latched DICR rise and generation-safely ORs `0x0008` into I_STAT. It
   never invokes a callback.
3. `PE_IRQ_ServicePendingForGeneration(captured_irq_generation)` executes
   the bounded retail CPU scanner and typed guest-identity dispatch. It
   never completes DMA.

The following states are therefore directly observable:

| Phase | CHCR | DICR flag 26 | I_STAT bit 3 | Callback |
| --- | --- | --- | --- | --- |
| before completion | busy | clear | clear | none |
| hardware complete | idle | set | clear | none |
| edge bridged | idle | set | set, even if masked | none |
| CPU acknowledged | idle | set | clear | DMA dispatcher entered |
| DMA acknowledged | idle | clear | clear | pump about to enter |

Masked source assertion is retained by the B1 I_STAT authority. Unmasking
makes it eligible later; neither assertion nor mask mutation dispatches it.

## Deterministic checkpoint and recursion guard

`PE_Port_ServiceDmaIrqCheckpoint()` is the suppressible host hardware
opportunity. Production invokes it immediately after `func_8006AD40()` and
before the sticky `PE_Port_ShouldStop()` check inside `func_8001220C`.

At checkpoint entry it captures exactly:

- the currently active DMA2 token, if any;
- the current IRQ generation.

It services the captured DMA token at most once, then calls the edge bridge,
then calls CPU pending service. It never loops or recaptures. A DMA issued by
a nested callback therefore receives a later hardware opportunity and cannot
complete recursively in the same checkpoint. An explicitly disabled
checkpoint admits no hardware progress. A nested boundary result is returned
directly so ordinary caller continuation is suppressed even when an older
sticky stop reason already exists.

There is no wall-clock dependency.

## CPU source dispatcher

The B2 CPU service is the execution-proven bounded translation of
`func_80073F00`. B1's `platform/pe_irq.c` remains the sole I_STAT/I_MASK and
generation authority; CPU identities remain the eleven 32-bit guest entries
at `D_800945E8`, and registered source mask `D_80094614` remains guest RAM.
There is no host callback-table mirror.

Initial eligibility is exactly:

```text
D_80094614 & I_STAT & I_MASK
```

Sources scan low-to-high, 0 through 10. The dispatcher publishes
`D_800945E6 = 1`, then for each selected source:

1. W0C-write `~(1 << source)` to I_STAT;
2. live-load `D_800945E8[source]`;
3. zero means no callback and scanning continues;
4. known source 3 identity `0x80074520` enters the typed DMA dispatcher;
5. every other nonzero, unbound identity stops at an honest typed indirect
   boundary after the acknowledgement.

After a normally returned snapshot it resamples all three authorities. On
ordinary terminal return, it executes the recovered watchdog behavior and
clears `D_800945E6` at the equivalent of retail `0x800740AC`. A nested
boundary returns before scan progression, watchdog work, or terminal cleanup,
so `D_800945E6` remains `1` exactly as guest execution would.

Generation mismatch is rejected before active-state publication or any
acknowledgement.

The pre-ResetCallback guard-zero path does not invent entry into the ordinary
scanner. It reports the recovered unexpected-interrupt diagnostic and stops
at the named `func_80074384` / BIOS B(17h) exception-return boundary before
publishing `D_800945E6` or acknowledging any source. The canonical initialized
path has guard one and is unaffected.

## Complete `func_80074520` translation

`PE_func_80074520_Dispatch()` carries an explicit returned/boundary result;
the public `void func_80074520(void)` preserves the retail ABI. Pending DMA
flags are exactly:

```text
(PE_GPU_ReadDICR() >> 24) & 0x7F
```

The scan is channels 0 through 6, low-to-high. For every selected snapshot
bit it:

1. rereads physical DICR;
2. computes the retail acknowledgement
   `physical & (0x00FFFFFF | (1 << (24 + channel)))`;
3. W1C-writes DICR before callback lookup;
4. live-loads guest identity `D_800956C0[channel]`;
5. acknowledges and continues for a zero slot;
6. dispatches bound channel-2 identity `0x80076EE4` through the typed pump;
7. stops honestly for every other nonzero, unbound identity;
8. advances/resamples only after a normal callback return.

Retained flags are selected independently of current enable bits. Multiple
flags dispatch in ascending channel order. Flags raised during a callback are
seen only after a normal returned pass. The literal diagnostic tail is kept;
the canonical acknowledged state does not enter it. It uses the exact retail
diagnostic strings and reads all seven MADRs in channel order through a
read-only routing surface: channel 2 comes from the existing GPU DMA2
authority, channel 4 from the existing SPU DMA authority, and currently
unrepresented channels retain their hardware-reset zero values. It neither
creates DMA workers nor fabricates callback state.

## Exact acknowledgement and callback order

Canonical delivery commits:

```text
DICR rising-edge bridge
< I_STAT source-3 W0C write 0xFFF7
< CPU callback identity 0x80074520
< DICR channel-2 W1C write 0x04840000
< DMA callback identity 0x80076EE4
< func_80076EE4_idle_pump boundary
```

The DMA callback observes:

```text
DMA2 CHCR busy clear
complete VRAM visible
I_STAT source 3 clear
DICR channel-2 flag clear
```

Unrelated I_STAT bits and unrelated DICR flags remain pending. CPU and DMA
callback identities are separate guest-backed tables and are never cast to
native pointers. Literal zero is the only skip value.

## Nested non-return contract and focused endpoint

The B53H typed pump API distinguishes an ordinary busy-path return from the
idle-path untranslated boundary. After genuine completion, the callback
identity `0x80076EE4` reaches `func_80076EE4_idle_pump`; it does not return a
fabricated zero.

The boundary propagates immediately through DMA dispatch, CPU dispatch, the
checkpoint, and `func_8001220C`. Already-performed acknowledgements remain
committed. No post-callback snapshot advance/resample/diagnostic, later CPU
source scan, watchdog, or terminal cleanup runs.

Focused before/after state is:

| State | Before | At B2 boundary |
| --- | --- | --- |
| DMA2 active | 1 | 0 |
| CHCR | `0x01000201` | `0x00000201` |
| stored DICR | `0x00840000` | `0x00840000` |
| physical DICR | `0x00840000` | `0x00840000` |
| I_STAT source 3 | 0 | 0, acknowledged |
| source-3 callback | `0x80074520` | unchanged |
| DMA2 callback | `0x80076EE4` | unchanged |
| dispatch active `D_800945E6` | 0 | 1 |
| producer / consumer | 1 / 0 | 1 / 0 |
| queued entry | valid | byte-identical |
| second worker calls | 0 | 0 |
| second DMA | absent | absent |

The complete first-image pixels are visible before the edge bridge and
callback. The focused boundary is `func_80076EE4_idle_pump`. The global
canonical continuing frontier remains separately measured at the existing
named `func_8006AD40_prefix_cut` (`0x8006AE50`) because the outer B50 prefix
still requests the earlier sticky host stop; bootstrap strict remains
`func_8007F72C` from `func_800698D4`.

## Reset and stale-event behavior

B53B's token generation and B1's IRQ generation remain distinct authorities.
A stale DMA token is rejected before data mutation. A stale IRQ generation is
rejected before consuming a current DICR edge, publishing dispatch-active, or
acknowledging status.

Full SDK reset coherently:

- advances IRQ generation and clears I_STAT/I_MASK;
- clears CPU source slots, registered mask, dispatch-active, and watchdog;
- clears all eight guest DMA callback identities;
- resets DMA/token state, stored DICR, derived level, and edge latch;
- prevents delivery after reset at completion, edge-bridge, and CPU-service
  boundaries.

ResetCallback itself does not advance generation. Its recovered
`func_800744D4` slice clears the eight DMA slots, writes literal zero to DICR,
then installs source 3. That DICR write clears controls but, by W1C semantics,
does not acknowledge preexisting flags. Full reset owns explicit stale-flag
clearing before the retail initializer runs.

## Independent evidence and regression coverage

`pc_port/tools/b53i_b2_oracle.py` verifies:

- exact retail executable SHA-1;
- all 96 words of `func_80074520` and its full body hash;
- 53 literal words spanning the CPU scan/ack and terminal-clear windows plus
  the full 116-word CPU body hash;
- explicit completion, bridge, and CPU-service phases;
- enabled, channel-disabled, and master-disabled completions;
- retained-flag/master and force-bit edges;
- masked pending followed by unmask;
- zero, bound, and unbound callback identities;
- multiple DMA flags in ascending order;
- nested idle-pump non-return;
- stale IRQ generation and reset between completion and service;
- explicit DMA token issue, exact-token completion, reset rejection, and
  rejection of an old token after a new post-reset issue;
- guard-zero exception-return boundary without acknowledgement;
- literal diagnostic entry and live channel-2/channel-4 MADR ordering.

Focused native coverage is grouped into 15 B2 tests. It additionally proves
VRAM/order visibility, bit 31 not stored, sticky edge consumption after a
fall, no high-level retrigger even after I_STAT is acknowledged while DICR
remains high, mask-independent edge assertion, exact acknowledgement/callback
trace, actual typed-pump entry telemetry proving no hidden busy-path callback
from completion or edge bridging, guard-zero inertia, live diagnostic MADR
ordering, queue inertia, `D_800945E6` normal/boundary outcomes, single-token admission,
stop-epoch propagation, unrelated-bit preservation, and no native callback
pointer storage.

Independent DICR, CPU-service, DMA-dispatch, reset, non-return, and final
adversarial reviews found no remaining HIGH or MEDIUM issue. Review-driven
fixtures specifically close silent busy-path early-callback, guard-zero
fallthrough, diagnostic-MADR, masked-edge, stale-edge, and completion-gating
false-positive cases.

The accepted B53B test set remains 15 tests after deliberate correction of
only its superseded completion-flag expectation. Final measured preservation
results are:

```text
native suite, normal                     548/548
native suite, fresh ASan/UBSan           548/548, zero diagnostics
focused B53I-B2, normal/sanitizer        15/15 each
retained B53I-B1, normal/sanitizer       8/8 each
corrected B53B, normal/sanitizer         15/15 each
retained B53H, normal/sanitizer          8/8 each
oracle/executable matrix                 44/44
retained oracle subset                   33/33
B53I-B2 oracle                           96 DMA words / 53 CPU-window words / 17 cases
B49 host-loop, normal/sanitizer          PASS / PASS
canonical strict, normal/sanitizer       exit 1 / exit 1
bootstrap strict, normal/sanitizer       exit 1 / exit 1
```

Preservation hashes are:

```text
framebuffer SHA-256  fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
real-data FNV-1a64   7D860391E1ED6C97
matching EXE SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
matching EXE SHA-256 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

## Next boundary

The next rung may translate only the recovered idle-DMA consumer beginning at
`0x80076F10`. It must preserve retail IRQ masking, DMA callback removal,
GPUSTAT readiness, queued worker issue, consumer advancement only after the
worker returns, and the single-opportunity completion rule. B2 does not enter
that body, deregister channel 2, invoke `0x80076664`, advance consumer, or
issue the second DMA.
