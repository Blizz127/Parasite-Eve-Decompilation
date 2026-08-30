# Phase 6E-B53A — `func_80076C34` GPU queue / DMA2 contract

## Verdict and scope

**Architecture decision: D — B53 DESIGN VERIFIED, IMPLEMENTATION DEFERRED.**

This is the audit/design half of B53. It makes no production-source change,
does not translate `func_80076C34` or any deeper participant, and does not
invent GPUSTAT or DMA completion. The recommended B53B implementation is the
bounded form of option C: one native GPU/DMA2 platform authority underneath
translated retail queue logic. The retail ring remains guest-backed; there
must not be a second host queue.

The audit did not enter `func_800718D0`, `func_80030894`, or
`func_801909B4`.

## Provenance, isolation, and canonical Disc 1 gate

The pre-edit collision guard was clean:

| Item | Exact result |
|---|---|
| checkout | `/home/blizz/dev/parasite-eve-port-black` |
| branch | `phase6e-b-provider-frontier` |
| B52 HEAD | `a9599d011b363d8b49954e475e9be7c4576d30e7` |
| B51 parent | `9c494f0ac09313681a6a99ebc3a6a04ed88eb7fc` |
| B50 corrective ancestor | `37990cf5fe5859285e2718415231eb9961cf353b` |
| B49 | `33042dba2975079175da0eb01c46d650e2e842a6` |
| status | clean; porcelain empty |

The provisional B50 commit `c34cd75` is also in the ancestry, immediately
before the corrective B50 commit. There was no Git lock, foreign process with
a cwd inside this checkout, compiler/test/emulator process, or other active
AI agent in the checkout.

The required canonical image exists and was used read-only:

```text
/home/blizz/dev/parasite-eve/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin
size:   495531120 bytes
sha256: 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
```

Fresh isolated normal and ASan/UBSan builds both passed the retained B49
bounded-run mechanism against that Disc 1. Continuing strict execution in
both builds exited 1 at one presentation and one outer main iteration. GDB
stopped at `Bootstrap_ReturnInt` before the fatal exit and recorded:

```text
main_iters=1 presents=1 vsyncs=0 drawsyncs=0
symbol="func_80076C34" caller="func_8007506C"
```

The complete continuing-strict fatal text was identical in the two builds:

```text
[DISC] opened '/home/blizz/dev/parasite-eve/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin' (210685 user sectors)
[DISC] boot executable loaded into guest RAM
[TRACE 0001] native_executable_start
[TRACE 0002] call_func_8001220C
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80076C34
       called from: func_8007506C
```

Bootstrap strict also exited 1 and remained:

```text
[TRACE 0001] native_executable_start
[TRACE 0002] call_func_8001220C
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_8007F72C
       called from: func_800698D4
```

The retained harness additionally reproved the one-frame and prefix runs at
one frame / one iteration and the canonical framebuffer SHA-256
`fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb`.

## Retail identity and body proof

`func_80076C34` is a nonmatching assembly body with no C implementation:

| Property | Exact value |
|---|---|
| executable start | `0x80076C34` |
| executable last byte | `0x80076EE3` |
| exclusive end | `0x80076EE4` |
| executable file offset | `0x67434` |
| size | `0x2B0` / 688 bytes |
| instruction count | 172 |
| live split | `asm/disc1/66B54.s` |
| map | `build/disc1.map` |
| body SHA-256 | `b3686b34851b08fa3bb0097263caf59519056417b593606b7e7a59c155b1e508` |
| executable SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |

All 172 assembly annotation byte-quads were compared to the 688 raw bytes at
file offset `0x67434`; the comparison was exact. Appendix A records every
quad. The executable used for that comparison independently hashes to the
accepted SHA-1 above.

The public SDK mapping is not guessed from address order. B52 string-proved
`func_8007506C` as `LoadImage`; the static libgpu table maps its dispatcher to
`func_80076C34` and its worker to `func_80076664`. The embedded table version
string begins `$Id: sys.c,v 1.140 1998/01/12 07:52:27 noda Exp $`. No local
Psy-Q library with internal symbols was available, so this report uses the
proven role “libgpu queue submit/dispatch” rather than inventing an internal
source name. Sony's Psy-Q reference independently specifies that `LoadImage`
is non-blocking, returns its libgpu queue position, and is completed through
`DrawSync`: [Run-Time Library Reference 4.4](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Libref.pdf).

The current native boundary remains the B52
`Bootstrap_ReturnInt4Indirect` call in
`pc_port/game/boot/func_8007506C_port.c`; no matching or translated
`func_80076C34` body exists.

## ABI, control flow, calls, and returns

The retail ABI is:

```c
int func_80076C34(
    int (*worker)(void *argument, uint32_t auxiliary), /* a0 */
    void *argument,                                    /* a1 */
    int copy_bytes,                                    /* a2 */
    uint32_t auxiliary);                               /* a3 */
```

All four pointer-looking retail values are 32-bit guest values. `a0` is not
a completion callback: it is the command-issue worker. The dispatcher calls
it as `worker(argument, auxiliary)` either synchronously or from the queue
pump, ignores its return, and only then advances the consumer. The distinct
drawing-completion callback lives at `D_80095758`.

When `copy_bytes != 0`, the queue path copies
`trunc_toward_zero(copy_bytes / 4)` words into the entry. Legitimate callers
use 0, 8, or 64 bytes. MoveImage's 20-byte packet is already persistent and
uses the no-copy shim. There is no retail length check against the 84-byte
inline payload. When `copy_bytes == 0`, the original guest pointer is stored.

The exact control transfers and delay slots are:

| PC | Transfer | Target / condition | Delay slot |
|---:|---|---|---|
| `76C58` | `jal` | `func_800773D0` | `s2 = a3` |
| `76C60` | `j` | `76C80` | `nop` |
| `76C68` | `jal` | `func_80077404` | `nop` |
| `76C70` | `bnez` | timeout -> `76EC8` | `v0 = -1` |
| `76C78` | `jal` | `func_80076EE4` | `nop` |
| `76C98` | `beq` | full -> `76C68` | `nop` |
| `76CA0` | `jal` | `func_80073E10` | `a0 = 0` |
| `76CC0` | `beqz` | uninitialized -> direct | `D_80095754 = 1` |
| `76CDC` | `bne` | nonempty -> queue | `nop` |
| `76CFC` | `bnez` | DMA2 busy -> queue | `nop` |
| `76D0C` | `bnez` | DrawSync callback set -> queue | `nop` |
| `76D2C` | `beqz` | GPUSTAT not ready -> `76D20` | `nop` |
| `76D38` | `jalr` | `s3` worker | `a1 = s2` |
| `76D48` | `jal` | `func_80073E10` restore | `nop` |
| `76D50` | `j` | return | `v0 = 0` |
| `76D60` | `jal` | `func_80073CF4` | `a0 = 2` |
| `76D68` | `beqz` | no copy -> `76E14` | `a2 = 0` |
| `76D80` | `bgez` | signed divide adjust | `nop` |
| `76D94` | `beqz` | copy done -> `76DD0` | `a0 = index * 4` |
| `76DC8` | `j` | copy loop -> `76D80` | `v0 = copy_bytes` |
| `76E0C` | `j` | entry tail -> `76E38` | store inline guest pointer |
| `76E9C` | `jal` | `func_80073E10` restore | store new producer |
| `76EA4` | `jal` | `func_80076EE4` | `nop` |
| `76EDC` | `jr` | return | restore stack |

The only indirect call is the worker at `0x80076D38`. Direct callees are
`func_800773D0`, `func_80077404`, `func_80076EE4`, `func_80073E10`, and
`func_80073CF4`.

Return paths are exact:

| Path | Return |
|---|---:|
| ring-full wait times out | `-1` |
| direct worker path | `0`, even if the worker returned `-1` |
| queued path after opportunistic pump | `(producer - consumer) & 0x3F` |

Thus a worker error is not forwarded. The public result is queue position or
the dispatcher's own timeout failure.

## Exhaustive executable caller census

There is one direct `jal func_80076C34`, inside `func_80076C10`, and eight
executable `jalr` sites through immutable `jtb[2]`. No other executable site
loads that dispatch entry. B54K-R corrected this audit's former off-by-one
jump-table interpretation: MoveImage is itself the eighth `jtb[2]` caller;
it does not traverse `func_80076C10`.

| Public caller / site | `a0` worker | `a1` | `a2` | `a3` | Call delay slot | Result use |
|---|---:|---|---:|---|---|---|
| ClearImage `74FB0` | `80076434` | caller RECT | 8 | `b<<16 | g<<8 | r` | forms `a3` | forwarded |
| ClearImage2 `75048` | `80076434` | caller RECT | 8 | `80000000 | b<<16 | g<<8 | r` | forms `a3` | forwarded |
| LoadImage `750B0` | `80076664` | transient RECT | 8 | source guest address | `a3 = s1` | forwarded |
| StoreImage `75110` | `800768A0` | transient RECT | 8 | destination guest address | `a3 = s1` | forwarded |
| DrawOTag `7540C` | `80076B98` | OT guest pointer | 0 | 0 | `a3 = 0` | forwarded |
| PutDrawEnv `754B0` | `80076B98` | draw packet at env+`1C` | 64 | 0 | `a3 = 0` | ignored; wrapper returns env |
| DrawOTagEnv `75588` | `80076B98` | draw packet at env+`1C` | 64 | 0 | `a3 = 0` | ignored |
| MoveImage `751C4` | `80076B98` | persistent packet `800957E4` | 20 | 0 | `a3 = 0` | forwarded |
| `func_80076C10` `76C1C` | incoming worker | incoming pointer | 0 | incoming `a2` | `a2 = 0` | forwarded |

`D_80095744` contains `0x80095704`. MoveImage loads the dispatch target from
that base plus 8 (`D_8009570C = func_80076C34`) and the worker from plus
`0x18` (`D_8009571C = func_80076B98`). At `0x800751C4` it therefore calls
the dispatcher directly with the persistent packet `D_800957E4`,
`a2=0x14`, and `a3=0`. MoveImage returns `-1` before submission for zero
width or height; otherwise it forwards the dispatcher result.

Every site has the same state-conditioned blocking contract described below.
For equal guest queue/MMIO state and equal arguments, the transition is
repeatable. ClearImage, ClearImage2, LoadImage, and StoreImage never retain
their transient RECT: they either consume it directly or copy it first.

The B52 call is exactly:

```text
a0 = 0x80076664
a1 = pointer to transient 8-byte RECT
a2 = 8
a3 = 32-bit source guest address
```

## Software ring and persistent state

The ring occupies `0x800BD030..0x800BE830` exclusive. It has 64 physical
entries of 96 (`0x60`) bytes:

| Entry offset | Width | Meaning | Producer | Consumer |
|---:|---:|---|---|---|
| `+00` | 4 | worker guest function address | dispatcher | pump |
| `+04` | 4 | worker argument guest address | dispatcher | pump |
| `+08` | 4 | auxiliary argument | dispatcher | pump |
| `+0C..+5F` | 84 | 21-word inline payload | dispatcher copy loop | selected worker |

`entry(i) = 0x800BD030 + i * 0x60`. A copied argument pointer is exactly
`entry(i)+0x0C`; a no-copy argument is the original guest address. There is
no stored length or ownership flag because the worker identity defines the
payload.

| Global | Width | Meaning / writers |
|---:|---:|---|
| `D_80095874` | 4 | producer; dispatcher, init, timeout reset |
| `D_80095878` | 4 | consumer; pump, init, timeout reset |
| `D_8009587C` | 4 | I_MASK saved by dispatcher |
| `D_80095880` | 4 | I_MASK saved by pump |
| `D_80095884` | 4 | I_MASK saved by init/timeout recovery |
| `D_80095888` | 4 | deadline `VSync(-1)+0xF0` |
| `D_8009588C` | 4 | software wait-poll counter |
| `D_80095754` | 4 | work-submitted marker; submit sets, completion clears |
| `D_80095758` | 4 | DrawSync completion callback guest address |

Both indices are always masked with `0x3F`. Empty is `producer==consumer`.
Full is `((producer+1)&0x3F)==consumer`; one ring slot is reserved, so there
can be 63 queued entries. The pump removes an entry immediately after its
worker issues it, even when DMA2 remains active. Therefore the SDK's 64
outstanding operations are one in-flight command plus 63 queued requests;
the 65th blocks. This independently matches Sony's queue description in the
[Psy-Q Run-Time Library Overview](https://psx.schnappy.xyz/sdk/Psy-Q/DOCS/LIBOVR46.PDF).

Pending count is `(producer-consumer)&0x3F`. It excludes the command already
owned by DMA2. DrawSync compensates by returning at least 1 when the ring is
empty but DMA2 or the GPU readiness state is busy.

`func_80077144` initializes consumer and producer to zero under I_MASK=0.
Modes 0 and 5 also clear all `0x1800` ring bytes; executable initial data is
zero. The timeout recovery in `func_80077404` discards every queued entry by
setting both indices to zero but does not invoke their workers.

### RECT and source lifetime

For LoadImage, `a2=8` copies both RECT words before producer publication.
The worker may clamp the copied `w/h` in place. After it has written the GP0
position/size words, the RECT is no longer needed; consumer advancement may
therefore release the slot even while DMA2 is active. The source address is
stored by value and remains live in DMA2 MADR/platform transfer state until
completion. A B53B implementation must never serialize a native stack
pointer into `entry+4`.

## Dispatcher and queue-pump lifecycle

`func_80076C34` implements mixed behavior, category **F**, combining B, C,
and E from the task's classification. It never waits for this specific
request to finish.

1. `func_800773D0` snapshots the timeout deadline and resets the software
   poll counter.
2. If the ring is full, it calls `func_80077404`, returns `-1` on timeout,
   otherwise calls the pump and retries the space test.
3. It exchanges I_MASK with zero and stores the previous mask. In the
   branch delay slot at `0x80076CC4`, it sets `D_80095754=1`.
4. It takes the direct path when GPU initialization byte `D_8009574D` is
   zero. When initialized, direct execution requires all of: ring empty,
   DMA2 CHCR bit 24 clear, and no DrawSync callback at `D_80095758`.
5. Direct execution tight-polls GPUSTAT bit 26, calls the worker, restores
   I_MASK, and returns 0. The worker may have started asynchronous DMA2.
6. Otherwise it registers `func_80076EE4` in DMA callback slot 2, writes the
   queue entry (copying when requested), increments producer, restores
   I_MASK, calls the pump opportunistically, and returns pending count.

`func_80076EE4` is the sole consumer:

1. If DMA2 CHCR bit 24 is set on entry, it returns 1 without masking
   interrupts or changing the queue.
2. It exchanges I_MASK with zero. If empty or DMA2 became busy, it restores
   I_MASK.
3. While nonempty and DMA2 idle, it checks whether the current entry is the
   last. If it is last and no DrawSync callback exists, it unregisters DMA
   slot 2 before issue; no IRQ is needed merely to drain an otherwise empty
   queue.
4. It tight-polls GPUSTAT bit 26, invokes `worker(entry.arg, entry.aux)`,
   ignores the worker result, and advances consumer immediately.
5. If more entries remain and DMA2 is still idle, it repeats in the same
   invocation. If the worker started DMA2, it stops and restores I_MASK.
6. When the ring is empty and DMA2 idle, if both the submitted marker and
   DrawSync callback are nonzero, it clears the marker before invoking the
   callback with no arguments.
7. Normal return is pending count; the initial DMA-busy fast path returns 1.

The exact LoadImage phase sequence is consequently:

```text
request -> space check -> direct decision or copied ring entry
        -> producer publication -> opportunistic pump
        -> GPUSTAT-ready check -> func_80076664 issues GP0 prefix
        -> optional DMA2 start -> consumer advances -> dispatcher returns
        -> hardware DMA completion event clears busy and raises DMA IRQ
        -> DMA dispatcher acknowledges DICR channel 2 -> pump re-entry
        -> next entry, or marker clear then DrawSync callback
```

Entry construction and producer publication are synchronous. Issue may be
opportunistic in the submitting call or deferred. DMA completion, subsequent
queue issue, and completion callback may be deferred. Ring-full waiting is
bounded; the direct and pump GPUSTAT tight loops themselves have no software
timeout. The LoadImage worker has its own bounded readiness loop.

## Callback and DMA interrupt contract

`func_80074CC8` (DrawSyncCallback) only swaps `D_80095758` and returns the
previous function. It never invokes the function immediately. The pump calls
the installed function with no arguments only after ring empty and DMA2
inactive, clearing `D_80095754` first.

Retail ResetCallback initialization installs `func_80074520` on interrupt
source 3 and returns `func_800746A0` as the DMA-channel callback setter.
Registering pump slot 2 performs:

```text
D_800956C0[2] = 0x80076EE4
DICR = (DICR & 0x00FFFFFF) | bit23 | bit18
I_MASK |= bit3                         (DMA interrupt source)
```

On DMA2 completion, hardware clears CHCR bit 24, sets the DMA channel-2
completion condition (DICR flag bit 26 and the surrounding DMA interrupt),
and `func_80074520` reads `(DICR>>24)&0x7F`. It acknowledges channel 2 with a
W1C write containing bit 26 while preserving the lower 24 configuration
bits, then invokes `D_800956C0[2]`. The pump therefore observes DMA idle
before issuing the next command or invoking DrawSyncCallback.

If the last entry was issued with no DrawSync callback, the pump had already
removed slot 2's enable bit. DMA completion is then observed by polling
(not by a callback needed for queue progress). If more work or a completion
callback exists, slot 2 stays registered and completion re-enters the pump.

## GPU MMIO map and exact operations

The retail pointer tables prove these physical addresses:

| Register | Address | Width |
|---|---:|---:|
| GP0 read/write | `1F801810` | 32 |
| GP1 write / GPUSTAT read | `1F801814` | 32 |
| DMA2 MADR | `1F8010A0` | 32 |
| DMA2 BCR | `1F8010A4` | 32 |
| DMA2 CHCR | `1F8010A8` | 32 |
| DPCR | `1F8010F0` | 32 |
| DICR | `1F8010F4` | 32 |
| I_STAT | `1F801070` | 16 |
| I_MASK | `1F801074` | 16 |

### Dispatcher, pump, and DrawSync reads

| Instruction PC | Operation | Derivation / surrounding control |
|---:|---|---|
| `76CF0` | R32 DMA2 CHCR | mask `01000000`; busy queues instead of direct |
| `76D20` | R32 GPUSTAT | mask `04000000`; branch back while zero |
| `76EFC` | R32 DMA2 CHCR | initial busy fast return 1 |
| `76F40` | R32 DMA2 CHCR | stop before consuming if busy after I_MASK=0 |
| `76FA4` | R32 GPUSTAT | initial bit-26 test before worker |
| `76FB8` | R32 GPUSTAT | unbounded retry until bit 26 set |
| `77080` | R32 DMA2 CHCR | after consumer advance; continue only if idle |
| `770CC` | R32 DMA2 CHCR | completion callback requires idle |
| `7730C` | R32 DMA2 CHCR | blocking DrawSync waits while bit 24 set |
| `7732C` | R32 GPUSTAT | blocking DrawSync waits while bit 26 clear |
| `77380` | R32 DMA2 CHCR | polling DrawSync returns at least 1 if busy |
| `773A0` | R32 GPUSTAT | polling DrawSync returns at least 1 if not ready |

`func_80073E10`, called around submit and pump mutation, performs the actual
I_MASK operations at `0x80073E1C` (R16) and in the `jr` delay slot at
`0x80073E24` (W16 supplied mask). Installing interrupt source 3 in
`func_800740D0` reads I_MASK at `0x80074130`, temporarily writes zero at
`0x80074134`, and writes the updated mask at `0x800741F0`.

### LoadImage worker writes and reads

| Instruction PC | Operation | Exact value / source |
|---:|---|---|
| `76758`, `7678C` | R32 GPUSTAT | mask bit 26; retry calls bounded wait |
| `767AC` | W32 GP1 | `04000000` (GPU DMA direction off) |
| `767BC` | W32 GP0 | `01000000` (clear command cache) |
| `767D4` | W32 GP0 | `A0000000` CPU->VRAM image command |
| `767E8` | W32 GP0 | RECT x/y word |
| `767FC` | W32 GP0 | clamped RECT w/h word |
| `76828` | W32 GP0 | each CPU remainder source word, loop delay slot |
| `76840` | W32 GP1 | `04000002` (DMA CPU->GPU) |
| `76850` | W32 DMA2 MADR | source after CPU remainder |
| `76864` | W32 DMA2 BCR | `(blocks<<16) | 0010` |
| `76874` | W32 DMA2 CHCR | `01000201` |

`s5` is initialized to zero and never changed, so the dormant B0 selection
at `0x800767D0` is not taken in this worker; A0 is the actual command.

### Reset and timeout recovery

| Instruction PC | Operation | Exact effect |
|---:|---|---|
| `771C0`, `77220` | W32 DMA2 CHCR | `00000401`, idle linked-list configuration |
| `771D4/771E0`, `77230/7723C` | RMW32 DPCR | OR `00000800`, enable channel 2 |
| `771F4` | W32 GP1 | 0, in memset call delay slot |
| `7724C`, `7725C` | W32 GP1 | `02000000`, then `01000000` |
| `77468` | R32 GPUSTAT | dead diagnostic read, still an MMIO read |
| `77484` | R32 DMA2 MADR | timeout diagnostic |
| `77498` | R32 GPUSTAT | timeout diagnostic |
| `7749C` | R32 DMA2 CHCR | timeout diagnostic |
| `774DC` | W32 DMA2 CHCR | `00000401` |
| `774EC/774F8` | RMW32 DPCR | OR `00000800` |
| `77508`, `77518` | W32 GP1 | `02000000`, then `01000000` |

The timeout printf receives pending count, GPUSTAT, CHCR, and stack-passed
MADR in that order. BCR is not logged.

### DICR and interrupt status

| Instruction PC | Operation | Exact effect |
|---:|---|---|
| `74504` | W32 DICR | zero during DMA dispatcher init |
| `74548` | R32 DICR | pending flags `(value>>24)&7F` |
| `745A0/745AC` | R/W32 DICR | acknowledge selected channel flag W1C |
| `745E4` | R32 DICR | resample pending flags |
| `74608`, `74620` | R32 DICR | post-dispatch diagnostic checks |
| `746DC/746FC` | R/W32 DICR | install: lower 24 + bit23 + channel enable |
| `74718/7473C` | R/W32 DICR | remove: keep bit23, clear channel enable |

No per-request B53 body directly reads or writes I_STAT. ResetCallback writes
I_STAT during global interrupt initialization; ordinary DMA2 delivery reaches
the executable through interrupt source 3. A native provider may collapse
the kernel's I_STAT dispatch, but it must preserve DICR enable/pending/ack
and callback ordering rather than calling the pump at DMA issue time.

## GPUSTAT semantics

The mask is derived from every retail instruction before naming the bit:
`lui 0x0400` produces `0x04000000`, bit 26. On this GPU it is the status
condition “ready to receive a GP0 command word.” All readiness branches treat
set as ready.

| Poll | Behavior while clear | Useful CPU work / queue mutation |
|---|---|---|
| dispatcher direct `76D20` | unbounded tight read loop | none / none |
| pump `76FA4..76FB8` | unbounded tight read loop | none / none |
| LoadImage worker `76758..76798` | calls `func_80077404`, then rereads | VSync query and timeout accounting; no queue change |
| DrawSync(0) `7732C` | calls bounded wait and retries | timeout accounting; queue already empty |
| DrawSync(1) `773A0` | no loop; returns pending or 1 | none |

Queue state never changes inside a readiness wait. Hardware evolution is the
normal cause of progress; worker/DrawSync waits also have a fallback timeout.
The native layer currently models none of these states. A B53B platform must
carry bit-26 readiness and provide a deterministic transition/service hook;
it may not hard-code every read as ready. Tests must be able to hold the bit
clear and release it explicitly.

## DMA2 contract and completion lifecycle

For a clamped positive `w*h`, define:

```text
pixels         = w * h
transfer_words = ceil(pixels / 2)
blocks         = transfer_words / 16
remainder      = transfer_words - blocks * 16
transfer_bytes = transfer_words * 4
```

The worker first sends `remainder` words through CPU GP0 writes, advancing
the source address. If `blocks!=0`, it selects GPU DMA direction 2 and writes:

```text
MADR = source + remainder*4
BCR  = (blocks<<16) | 0x10
CHCR = 0x01000201
```

CHCR proves RAM-to-device direction (bit 0), request/block synchronization
(bit 9), and start/busy (bit 24). It is not linked-list mode. BCR is 16 words
per block and `blocks` blocks. DMA therefore transfers exactly the remaining
multiple of 16 words. The queue worker does not poll completion and returns
0 immediately after starting DMA.

Completion is a later hardware event. Before it, CHCR bit 24 remains set,
the dispatcher queues new requests, the pump returns 1, and DrawSync reports
busy. At the event, data completion precedes clearing busy/dispatching the
DMA2 callback. The DICR channel-2 callback re-enters the pump. If another
entry exists it is issued next; otherwise the DrawSync completion callback
may run. CPU never waits for this specific LoadImage inside
`func_80076C34`.

The reset value `0x00000401` is an idle linked-list channel configuration,
not an active transfer. DPCR OR `0x800` enables DMA channel 2 while preserving
its priority. DICR master bit 23 and channel enable bit 18 govern completion
interrupt delivery.

## Complete `func_80076664` LoadImage worker

`func_80076664` occupies `0x80076664..0x800768A0` exclusive, file offset
`0x66E64`, size `0x23C` / 143 instructions. Its ABI is:

```c
int func_80076664(RECT *rect, pe_addr_t source);
```

It is a command-issue worker, not a callback. It is invoked synchronously by
the dispatcher/pump and receives no later callback arguments.

1. It initializes the shared timeout deadline.
2. It clamps signed `w` to `[0,D_80095750]` and signed `h` to
   `[0,D_80095752]`, storing the results into its argument. ResetGraph(0)
   establishes limits 1024 and 512. It does not alter x or y.
3. It calculates `ceil(w*h/2)` 32-bit words with the exact signed rounding
   sequence. If the result is not positive it returns `-1` without GPU MMIO.
4. It waits for GPUSTAT bit 26, bounded through `func_80077404`.
5. It emits GP1/GP0 A0 command, x/y, w/h, and 0..15 CPU words.
6. For remaining full 16-word blocks it programs DMA2 as above.
7. It returns 0 without waiting for DMA2.

The GP0 image data are raw 16-bit framebuffer pixels packed two per 32-bit
source word, low halfword first. Odd pixel count consumes one complete final
source word; its padding halfword is not a destination pixel. Source must be
4-byte aligned because the CPU prefix uses `lw`, and the rounded source byte
range must remain valid through DMA completion.

The GPU receives an absolute framebuffer RECT. The worker performs no draw
area clipping and no x/y or x+w/y+h correction. The public Psy-Q contract
requires the destination to lie within `(0,0)..(1023,511)` and states that
clip/offset do not affect it. At the hardware level VRAM is 1024x512 16-bit
pixels and overflow wraps at the framebuffer edge rather than clipping;
Sony's hardware manual describes the 1 MiB VRAM, DMA access, dimensions, and
returned overflow: [Sony PlayStation Hardware Manual](https://manualzz.com/doc/25793104/sony-playstation-hardware-manual).
The B53B provider should mask/wrap X modulo 1024 and Y modulo 512
independently, while retaining the SDK's valid-input contract. Width/height
zero never reach GP0 through this worker; negative values clamp to zero and
return `-1`.

For a CPU-only transfer (`transfer_words<16`), all pixels have been supplied
before the worker returns. For a DMA transfer, complete visibility must occur
no later than the DMA completion event and before the completion callback.
The SDK provides no observation guarantee before DrawSync/completion, so a
bounded platform may stage the transfer atomically until that event; it must
not expose the completed image or call the callback at issue time.

## Timeout and blocking details

`func_800773D0` stores `VSync(-1)+240` and zeroes the software counter.
`func_80077404` returns 0 until either signed current VSync exceeds that
deadline or the prior software counter exceeds `0xF0000`. On timeout it:

1. prints queue/GPU/DMA diagnostics;
2. exchanges I_MASK with zero;
3. resets consumer and producer to zero;
4. writes idle DMA2 CHCR, enables DMA2 in DPCR;
5. writes GP1 acknowledge IRQ then reset command buffer;
6. restores I_MASK and returns `-1`.

DrawSync(0) first pumps until the ring is empty, then waits for DMA2 bit 24
to clear, then waits for GPUSTAT bit 26. DrawSync(1) snapshots pending count,
opportunistically pumps when nonzero, and returns pending count; if it was
zero but DMA2 is active or GPU not ready, it returns 1. These retail bodies
explain both non-blocking LoadImage and the observable completion barrier.

Final blocking classification:

- not A: it may do more than enqueue;
- B: yes, it opportunistically pumps;
- C: yes, it blocks only when the ring is full, with timeout;
- not D: it never waits for this specific command's DMA completion;
- E: yes, direct issue and pump wait on GPUSTAT readiness;
- therefore F: mixed behavior based on queue, DMA, initialization, callback,
  and GPU readiness state.

## Direct participant dependency map

| Function | Size / role | Hardware dependency | Native now | Required for B53B |
|---|---|---|---|---|
| `80076C34` | `2B0`, submit/direct/enqueue | GPUSTAT, CHCR, I_MASK | unresolved | yes |
| `80076EE4` | `260`, sole consumer/pump | GPUSTAT, CHCR, I_MASK, callbacks | none | yes |
| `80076664` | `23C`, LoadImage issue worker | GP0/GP1/DMA2 | none | yes |
| `800773D0` | `34`, wait deadline init | VSync(-1) | counter shim incompatible | yes |
| `80077404` | `144`, bounded wait/recovery | GPUSTAT/DMA2/DPCR/GP1 | none | yes |
| `80077294` | `13C`, DrawSync barrier/poll | queue/GPUSTAT/DMA2 | counter only | yes for acceptance |
| `80077144` | `150`, queue/GPU reset | DMA2/DPCR/GP1 | collapsed | yes for reset |
| `80073E10` | `18`, I_MASK exchange | I_MASK | none | represent atomically |
| `80073CF4` | `30`, DMA callback wrapper | indirect libetc table | none | yes/bounded replacement |
| `800744D4` | `4C`, DMA dispatcher init | DICR/IRQ source 3 | collapsed | reset integration |
| `80074520` | `180`, DMA IRQ dispatcher | DICR | none | event-equivalent path |
| `800746A0` | `AC`, per-channel DMA callback setter | DICR | none | channel 2 subset |
| `800740D0` | `148`, IRQ callback setter | I_MASK | collapsed | source-3 subset |
| `80074CC8` | `60`, DrawSyncCallback setter | guest callback state | none | callback tests |
| `80077A00` | `28`, restore pump as DMA2 callback | DICR via setter | none | only if break/continue path enters scope |

`func_80077548` (`A0` bytes) performs deeper GPU reset/version probing and is
called by init mode 0. Its reset-visible subset must be represented, but B53A
does not translate it. StoreImage and other workers are not required to
cross the execution-proven LoadImage frontier and must remain explicit
unsupported identities in a LoadImage-only B53B.

## Existing PC-port authority audit

| State / operation | Current authority and users | Reset | Fidelity finding |
|---|---|---|---|
| PSX VRAM | none | none | missing |
| host visible image | `host_framebuffer.c` 320x240 RGB array; ClearImage/Present/tests | `HostFB_Init` | presentation/test surface, not PSX VRAM |
| GP0 / GP1 / GPUSTAT | none | ResetGraph effects skipped/collapsed | missing |
| software GPU ring | retail addresses exist in guest RAM but no native consumer | RAM/executable load only | no lifecycle |
| primitive rasterization | only host ClearImage rectangle fill | HostFB init | convenience subset |
| LoadImage | B52 wrapper ends at strict boundary | none | no pixels transferred |
| StoreImage / MoveImage | no GPU provider | none | missing |
| DrawSync | `HostFB_DrawSync` increments a counter | HostFB init | no wait/status semantics |
| VSync | `HostFB_VSync` increments a counter for every mode and returns no value | HostFB init | cannot supply retail VSync(-1) query |
| DMA2 / DICR | none | ResetCallback collapses hardware | missing |
| GPU event ticking | none | none | missing |
| generic callbacks | guest-backed VBlank slots in `pe_callback.c` | callback init/reset | useful binding machinery, not DMA2 slots |
| asynchronous DMA precedent | `pe_spu_dma.c`, DMA4 + SPU RAM | `PE_Sdk_ResetState` | separate authority; pattern only |

There is therefore no faithful provider beneath a full retail translation.
The future GPU model must not reuse SPU RAM/DMA4 state and must not treat the
host RGB array as VRAM. VRAM should be the single 1024x512x16 authority; the
host framebuffer remains a projection/presentation surface. Existing direct
host ClearImage behavior will eventually need to update or route through
that same VRAM authority so it cannot diverge.

## Recommended B53B architecture (option C)

B53B should add one bounded deterministic GPU platform with these ownership
rules:

1. The 64x96 retail ring, indices, marker, DrawSync callback, and copied
   packets remain authoritative in guest RAM at their retail addresses.
   Translate the dispatcher/pump bookkeeping; do not mirror them in a host
   queue.
2. A single host GPU state owns 1024x512 16-bit VRAM, GP0 image-transfer
   parser state, GP1 DMA direction, GPUSTAT readiness, DMA2 MADR/BCR/CHCR,
   DPCR channel-2 enable, DICR channel-2 enable/pending, and one pending
   completion event.
3. DMA issue records guest addresses and transfer geometry only. It must not
   retain a host pointer and must not call the completion path during issue.
4. A controlled service operation performs data completion, makes completed
   VRAM content visible, clears CHCR busy, raises/acknowledges the modeled
   DMA2 event, and dispatches the pump. Data visibility precedes callback.
5. Service is deterministic and causal, never wall-clock based. Retail
   GPU/DMA status polls, VSync query/wait points, DrawSync, and host-safe loop
   continuation are valid explicit service opportunities. Tests must also
   be able to withhold and deliver an event directly. GPUSTAT readiness and
   DMA completion are distinct states.
6. Translate VSync(-1) as a query and VSync(0) as a deterministic VBlank
   step; the current void counter shim cannot be used by the retail timeout.
7. Invoke guest function identities through typed bindings/switches. Never
   cast a guest address to a host function pointer.
8. B52 passes a native transient RECT. Add a host-copy adapter that either
   consumes a local copy synchronously or copies its eight bytes into the
   guest ring before publication. Only a guest address may enter `entry+4`.
9. Validate source alignment and the full rounded guest range before native
   dereference/DMA scheduling. Invalid native inputs fail visibly and without
   host OOB access; valid retail behavior remains exact.
10. Reset cancels pending DMA/events and prevents stale callback delivery.
    GP1/reset does not imply clearing VRAM; host lifecycle initialization may
    clear it separately.

This is a minimal lifecycle model, not a cycle-accurate PSX emulator. It
preserves every observed ordering and wait condition without inventing a
second queue or immediate-success shortcut.

## Future B53B acceptance contract

Tests must be based on the recovered state machine and cover at least:

1. idle/no-callback direct LoadImage returns 0 and retains no RECT pointer;
2. empty-ring enqueue forced by a DrawSync callback copies RECT into its own
   entry before producer publication;
3. nonempty enqueue while DMA2 busy preserves order and returns pending;
4. 63 queued plus one in-flight operation makes the next request wait;
5. full-ring progress after an explicit DMA event and `-1` timeout/reset when
   progress is withheld;
6. producer and consumer wrap from slot 63 to slot 0;
7. copied RECT survives caller-stack destruction and is mutated only in its
   queue copy;
8. no native pointer or upper pointer bits enter 32-bit guest state under
   normal and sanitizer builds;
9. aligned valid source range, unaligned source, rounded odd-pixel range, and
   source-end bounds failure;
10. exact GP1/GP0 sequence A0, x/y, clamped w/h, and CPU remainder words;
11. transfer sizes below 16 words, exact multiples of 16, remainder+DMA, odd
    pixel count, zero/negative size, and maximum 1024x512;
12. GPUSTAT bit 26 held clear causes no queue mutation/issue, then explicit
    ready transition permits issue;
13. MADR/BCR/CHCR values and RAM-to-GPU block mode are exact;
14. issue alone leaves DMA busy and the complete VRAM result unavailable;
15. explicit event completes pixels before clearing/delivering callback;
16. consumer advances after issue, not after DMA completion; in-flight DMA
    is represented by DrawSync returning at least 1;
17. DMA completion pumps the next entry before final completion callback;
18. callback marker clears before callback and callback fires once only;
19. 1024x512 absolute addressing, independent X/Y wrapping, and no draw-area
    clipping;
20. repeated transfers remain ordered and deterministic;
21. reset with pending DMA cancels the event, clears queue indices, and
    prevents stale pump/application callback invocation;
22. DrawSync(0) blocks through queue, DMA, then GPU readiness; DrawSync(1)
    polls without blocking;
23. deterministic event delivery produces identical VRAM, callback order,
    queue state, framebuffer projection, and diagnostics across runs.

No B53A oracle was added. The raw-body comparison was sufficient and did not
model any invented hardware evolution.

## Appendix A — all 172 retail instruction byte-quads

These are file-order raw byte-quads, one per instruction, from
`0x67434..0x676E3`:

```text
d8ffbd27 1c00b3af 21988000 1000b0af 2180a000 1400b1af 2188c000 1800b2af
2000bfaf f4dc010c 2190e000 20db0108 00000000 01dd010c 00000000 95004014
ffff0224 b9db010c 00000000 0980023c 7458428c 0980033c 7858638c 01004224
3f004230 f3ff4310 00000000 84cf010c 21200000 0980043c 4c578424 0980013c
7c5822ac 01008390 01000224 14006010 080082ac 0980033c 7458638c 0980023c
7858428c 00000000 1e006214 00000000 0980023c 6058428c 00000000 0000428c
0001033c 24104300 16004014 00000000 0c00828c 00000000 12004014 00000000
0980033c 5458638c 0004043c 0000628c 00000000 24104400 fcff4010 00000000
21200002 09f86002 21284002 0980043c 7c58848c 84cf010c 00000000 b2db0108
21100000 0780053c e46ea524 3dcf010c 02000424 2a002012 21300000 0c80083c
3cd00825 21380002 21102002 02004104 00000000 03004224 83100200 2a10c200
0e004010 80200600 0000e58c 0400e724 0980033c 7458638c 0100c624 40100300
21104300 40110200 21104800 21208200 000085ac 60db0108 21102002 0980023c
7458428c 0980033c 7458638c 40200200 21208200 40210400 40100300 21104300
40110200 0c80033c 3cd06324 21104300 0c80013c 21082400 8edb0108 34d022ac
0980033c 7458638c 00000000 40100300 21104300 40110200 0c80013c 21082200
34d030ac 0980033c 7458638c 00000000 40100300 21104300 40110200 0c80013c
21082200 38d032ac 0980033c 7458638c 00000000 40100300 21104300 40110200
0c80013c 21082200 30d033ac 0980023c 7458428c 0980043c 7c58848c 01004224
3f004230 0980013c 84cf010c 745822ac b9db010c 00000000 0980023c 7458428c
0980033c 7858638c 00000000 23104300 3f004230 2000bf8f 1c00b38f 1800b28f
1400b18f 1000b08f 0800e003 2800bd27
```

## Production-change statement

No production source changed in B53A. This evidence document is the only
planned repository change; implementation and acceptance tests are deferred
to B53B.
