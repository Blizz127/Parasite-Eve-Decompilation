# PE-B54K-L — first `D_80093126` issue, E0 walk, and wait

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the next coherent group in `func_8006AD40`: issue the
first `D_80093126` range into `+0x188`, process every entry in the completed
E0 archive exactly once, and consume the new read's completion. It stops
before the first load from the newly completed `+0x188` archive.

## Retail identity and geometry

```text
executable SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
new VA range       [0x8006B16C,0x8006B220)
new file range     [0x0005B96C,0x0005BA20)
new size           0xB4 bytes / 45 words
new SHA-256        5d86f9b22d6c98cf5a33e780bf92c431097c02dc39a7a654533f87645c1fecd6
prior word         0x8006B168  move s0,zero
last included      0x8006B21C  lui v0,0x003F (poll-branch delay slot)
first excluded     0x8006B220  lw s4,0x188(s5)
prefix arithmetic  0x8006B220 - 0x8006AD40 = 0x4E0 bytes = 312 words
```

The exact retail word sequence divides into these owners:

```text
0x8006B16C..0x8006B198  materialize D_80093126, issue into +0x188,
                         retry immediate issue returns of -1, set s2=1
0x8006B19C..0x8006B200  once-only completed-E0 archive walk at +0x16C
0x8006B204..0x8006B21C  timeout descriptor rebuild, live completion poll,
                         positive-poll return through the once-only gate
```

Retail `D_80093126[0..1]={0x0219,0x021B}`, so this read is exactly two
2048-byte sectors. The archive walk is the same proven packed-header shape
used earlier in this function:

```text
base     = *(D_800B0CD8+0x16C)
metadata = base + *(u32 *)(base+4)
header   = *(u32 *)(metadata+0x28)
count    = header >> 22
entry    = base + (header & 0x003FFFFF)
stride   = 0x14
callee   = func_8006E1C0(entry, base)
```

At `0x8006B1FC`, `s0=1` records that the walk is complete. A timeout at
`0x8006B204` returns to descriptor setup at `0x8006B16C`, but retained `s0`
skips the walk. A positive poll returns to the same gate at `0x8006B19C`.
The branch at `0x8006B218` always executes its `lui v0,0x003F` delay slot;
that register carry is consumed by the next archive block, so the cut includes
the instruction while assigning it no invented guest effect.

## Native implementation

The native form preserves the two independent state owners:

```c
int entries_done = 0;

do {
    uint32_t start = PE_LoadU16(GA_D_80093126);
    uint32_t end = PE_LoadU16(GA_D_80093126 + 2u);
    status = func_8006E6A8(
        (int)(lba_base + start),
        PE_LoadU32(GA_D_800B0CD8 + 0x188u),
        (int)(end - start));
} while (status == -1);
status = 1;

for (;;) {
    if (!entries_done) {
        /* derive packed count/offset and call func_8006E1C0 per entry */
        entries_done = 1;
    }
    if (status == -1) {
        /* reissue only the same D_80093126 range */
        status = 1;
    }
    status = func_8006E7E8();
    if (status == 0)
        break;
}
```

The synchronous host provider naturally exercises completion. Retry and
positive-poll topology are proven from the retail words by the independent
oracle; no test-only provider state is planted.

## Focused tests

The positive test uses a 640-sector MODE2 in-memory disc because
`FX_PEIMG_LBA + 0x21B` lies beyond the prior fixture. It places a real
two-entry archive in the E0 disc range, preserves the K rung's keyed F0
archive, and canaries the new destination. It proves:

- exactly two additional `func_8006E1C0` image dispatches;
- ordered data addresses `E0_BASE+0x500`, then `E0_BASE+0x600`;
- exact 0x14 entry stride and packed count ownership;
- exactly two sectors written, with the byte at `+0x1000` untouched;
- final provider state at `+0x188`, cleared busy bits, and one frontier hit.

The negative test leaves the E0 archive count zero. It proves no entry call is
made while the same D_80093126 transfer, completion, and frontier still occur.

```text
TEST B54KL_6AD40_e0_counted_then_3126_completion... PASS
TEST B54KL_6AD40_empty_e0_skips_entry_walk... PASS
Results: 952 run, 2 passed, 0 failed, 950 skipped
```

## Oracle and gates

`pc_port/tools/b54kl_6ad40_3126_wait_oracle.py` imports no production code.
It authenticates the executable/window, compares all 45 words, and verifies
issue ABI, entry-loop shape, all control-flow edges, both boundary words, the
delay-slot carry, and prefix arithmetic.

```text
  OK retail identity and complete comparison: 45/45 words
  OK D_80093126 issue: +0x188, end-start, immediate -1 retry
  OK E0 walk: packed count/offset, func_8006E1C0, 0x14 stride, once
  OK wait topology: timeout rebuilds; positive skips completed walk
  OK cut: delay-slot lui included; next lw s4,0x188(s5)
  OK prefix arithmetic: 0x4E0 bytes / 312 words

B54K-L oracle: 6 check groups passed.
```

```text
normal:         Results: 952 run, 952 passed, 0 failed, 0 skipped
fresh ASan/UB:  Results: 952 run, 952 passed, 0 failed, 0 skipped
                SANITIZER_DIAGNOSTICS=0
```

The retail executable remains SHA-1 exact. No scheduler destination,
`m0360i` special case, or persistence bit was added.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_archive_cut
SEMANTIC_IMPLEMENTATION=verified_through_0x8006B220
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
