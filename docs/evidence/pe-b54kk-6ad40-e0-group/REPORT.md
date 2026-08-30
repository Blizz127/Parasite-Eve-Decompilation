# PE-B54K-K — `D_800930E0` issue, F0 lookups, and completion

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the next natural retail group in `func_8006AD40`. It
issues the five-sector `D_800930E0` range, resolves three keyed objects from
the already-completed F0 archive, consumes the live completion result, and
moves the strict production frontier to the `D_80093126` descriptor setup.

## Retail identity and cut geometry

```text
executable SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
new VA range       [0x8006B0D4,0x8006B16C)
new file range     [0x0005B8D4,0x0005B96C)
new size           0x98 bytes / 38 words
new SHA-256        d0f529015f25ed5164d48b005ba3d5f8de273a9d776b44f8128cec57391bed0c
prior word         0x8006B0D0  move s0,zero
first excluded     0x8006B16C  lui s1,%hi(D_80093126)
prefix arithmetic  0x8006B16C - 0x8006AD40 = 0x42C bytes = 267 words
```

Complete new retail window:

```text
8006b0d4: 3c118009  lui   s1,0x8009
8006b0d8: 263130e0  addiu s1,s1,0x30e0
8006b0dc: 2412ffff  li    s2,-1
8006b0e0: 8ea5016c  lw    a1,0x16c(s5)
8006b0e4: 96220000  lhu   v0,0(s1)
8006b0e8: 96260002  lhu   a2,2(s1)
8006b0ec: 02c22021  addu  a0,s6,v0
8006b0f0: 0c01b9aa  jal   func_8006E6A8
8006b0f4: 00c23023  subu  a2,a2,v0
8006b0f8: 1052fff9  beq   v0,s2,0x8006b0e0
8006b0fc: 00000000  nop
8006b100: 24120001  li    s2,1
8006b104: 2411ffff  li    s1,-1
8006b108: 16000010  bnez  s0,0x8006b14c
8006b10c: 3c05c4b5  lui   a1,0xc4b5
8006b110: 34a5ba04  ori   a1,a1,0xba04
8006b114: 8ea4014c  lw    a0,0x14c(s5)
8006b118: 0c01b926  jal   func_8006E498
8006b11c: 24100001  li    s0,1
8006b120: 3c05caad  lui   a1,0xcaad
8006b124: 8ea4014c  lw    a0,0x14c(s5)
8006b128: 34a50704  ori   a1,a1,0x0704
8006b12c: 0c01b926  jal   func_8006E498
8006b130: aea2011c  sw    v0,0x11c(s5)
8006b134: 3c055eaf  lui   a1,0x5eaf
8006b138: 8ea4014c  lw    a0,0x14c(s5)
8006b13c: 34a56804  ori   a1,a1,0x6804
8006b140: 0c01b926  jal   func_8006E498
8006b144: aea20120  sw    v0,0x120(s5)
8006b148: aea20124  sw    v0,0x124(s5)
8006b14c: 1251ffe1  beq   s2,s1,0x8006b0d4
8006b150: 00000000  nop
8006b154: 0c01b9fa  jal   func_8006E7E8
8006b158: 00000000  nop
8006b15c: 00409021  move  s2,v0
8006b160: 1640ffe9  bnez  s2,0x8006b108
8006b164: 00000000  nop
8006b168: 00008021  move  s0,zero
```

The retail table values are `D_800930E0={0x79,0x7E}` and
`D_800930F0={0xC8,0xCB}`. Therefore E0 is exactly five 2048-byte sectors and
F0 is exactly three. The three calls use the completed F0 destination at
`*(D_800B0CD8+0x14C)` and store results as follows:

```text
key 0xC4B5BA04 -> *(D_800B0CD8+0x11C)
key 0xCAAD0704 -> *(D_800B0CD8+0x120)
key 0x5EAF6804 -> *(D_800B0CD8+0x124)
```

`s0` changes from zero to one in the first lookup's delay slot. Consequently
positive polls return through `0x8006B108` without repeating the lookups. A
timeout (`s2 == -1`) returns to `0x8006B0D4`, rebuilds the descriptor, and
reissues only E0. A zero poll exits and clears `s0`.

## Native implementation

The native state machine preserves those owners and does not assign a poll
result or mutate scheduler state:

```c
int lookups_done = 0;

do {
    uint32_t start = PE_LoadU16(GA_D_800930E0);
    uint32_t end = PE_LoadU16(GA_D_800930E0 + 2u);
    status = func_8006E6A8(
        (int)(lba_base + start),
        PE_LoadU32(GA_D_800B0CD8 + 0x16Cu),
        (int)(end - start));
} while (status == -1);
status = 1;

for (;;) {
    if (!lookups_done) {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x14Cu);
        lookups_done = 1;
        PE_StoreU32(GA_D_800B0CD8 + 0x11Cu,
                    func_8006E498(base, 0xC4B5BA04u));
        PE_StoreU32(GA_D_800B0CD8 + 0x120u,
                    func_8006E498(base, 0xCAAD0704u));
        PE_StoreU32(GA_D_800B0CD8 + 0x124u,
                    func_8006E498(base, 0x5EAF6804u));
    }
    if (status == -1) {
        /* reissue the same E0 range, then restore retail status 1 */
        /* ... */
    }
    status = func_8006E7E8();
    if (status == 0)
        break;
}
```

The host CD provider is synchronous, so runtime tests naturally cover the
completion edge. The independent oracle proves the unreachable-on-host
timeout and positive-poll topology; no test hook falsifies provider timing.

## Focused tests

The shared synthetic ISO has only 64 sectors, but these retail ranges reach
relative sector `0xCB`. B54K-K therefore grows only its two fixtures to 256
valid MODE2 sectors, preserving all original fixture bytes. It collapses the
three already-tested predecessor ranges to zero length while retaining the
exact E0 and F0 ranges. This isolates the new group without letting an
unrelated predecessor read replace B50's crafted guest archive.

`B54KK_6AD40_e0_archive_lookups_and_completion` places a real three-entry
archive in the on-disc F0 range. It proves all three returned guest pointers,
the exact five-sector E0 write with an untouched end sentinel, final provider
destination/state, no replay of earlier image work, and the sole new frontier.

`B54KK_6AD40_empty_f0_clears_stale_results` supplies an empty F0 archive after
poisoning all three output words. It proves misses write zero rather than
retaining stale results, E0 still completes, and the same frontier is reached.

```text
TEST B54KK_6AD40_e0_archive_lookups_and_completion... PASS
TEST B54KK_6AD40_empty_f0_clears_stale_results... PASS
Results: 950 run, 2 passed, 0 failed, 948 skipped
```

## Independent oracle and gates

`pc_port/tools/b54kk_6ad40_e0_group_oracle.py` imports no production code.
It authenticates the executable and window hash, compares every word, decodes
all calls and branch targets, verifies exact keys/stores, and closes both
boundaries and the whole-prefix arithmetic.

```text
  OK retail identity: exact executable and 38-word window hash
  OK complete retail comparison: 38/38 words
  OK E0 issue: +0x16C, end-start sectors, -1 retries issue only
  OK three lookups: exact keys, +0x11C/+0x120/+0x124, once per group
  OK wait topology: timeout rebuilds; positive repolls; zero exits
  OK cut geometry: prior clear-s0; next lui s1,%hi(D_80093126)
  OK prefix arithmetic: 0x42C bytes / 267 words

B54K-K oracle: 7 check groups passed.
```

Normal suite:

```text
Results: 950 run, 950 passed, 0 failed, 0 skipped
```

Fresh out-of-tree ASan/UBSan suite:

```text
Results: 950 run, 950 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

The retail executable remains SHA-1 exact. No scheduler destination,
`m0360i` special case, or persistence bit was added; scheduler provenance
remains `NEEDS_ARTIFACT` independently of this production advancement.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_cut
SEMANTIC_IMPLEMENTATION=verified_through_0x8006B16C
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
