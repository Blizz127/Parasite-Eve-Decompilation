# PE-B54K-J — `D_800930F0` completion/reissue gate

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the eight retail words after the completed
`func_80030894` call. It consumes the live `D_800930F0` completion result,
reissues only that read on timeout, and moves the strict production frontier
to the setup for the next descriptor, `D_800930E0`.

## Retail identity and cut geometry

```text
executable SHA-1   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
new VA range       [0x8006B0B4,0x8006B0D4)
new file range     [0x0005B8B4,0x0005B8D4)
new size           0x20 bytes / 8 words
new SHA-256        a8aa4d36f4fc116a2a4b3f5ca9ac8338beb625d749bde2ed82cc72b512993505
prior word         0x8006B0B0  nop (func_80030894 jal delay slot)
first excluded     0x8006B0D4  lui s1,%hi(D_800930E0)
prefix arithmetic  0x8006B0D4 - 0x8006AD40 = 0x394 bytes = 229 words
```

Complete new retail window:

```text
8006b0b4: 1251ffeb  beq   s2,s1,0x8006b064
8006b0b8: 00000000  nop
8006b0bc: 0c01b9fa  jal   0x8006e7e8
8006b0c0: 00000000  nop
8006b0c4: 00409021  move  s2,v0
8006b0c8: 1640fff3  bnez s2,0x8006b098
8006b0cc: 00000000  nop
8006b0d0: 00008021  move  s0,zero
```

The upstream branch destinations prove the loop ownership:

```text
0x8006B064  issue D_800930F0 into *(D_800B0CD8+0x14C)
0x8006B090  s2 = 1
0x8006B098  if (s0 != 0) branch to 0x8006B0B4
0x8006B0A0  func_800718D0(*(D_800B0CD8+0x180)) when s0 == 0
0x8006B0AC  func_80030894()
```

Thus `s2 == -1` at `0x8006B0B4` returns to the F0 issue, but retained
`s0 == 1` skips both `func_800718D0` and `func_80030894` afterward. A
positive poll returns through `0x8006B098` to the status gate. A zero poll
sets `s0 = 0` and advances into the next archive group.

## Native implementation

The translated structure is the retail state machine, with no new provider
or planted state:

```c
status = 1; /* retail 0x8006B090 after the initial F0 issue */
func_800718D0(PE_LoadU32(GA_D_800B0CD8 + 0x180u));
func_80030894();

for (;;) {
    if (status == -1) {
        do {
            uint32_t start = PE_LoadU16(GA_D_800930F0);
            uint32_t end = PE_LoadU16(GA_D_800930F0 + 2u);
            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x14Cu),
                (int)(end - start));
        } while (status == -1);
        status = 1;
    }
    status = func_8006E7E8();
    if (status == 0)
        break;
}
```

The host CD provider is synchronous and canonically returns zero here.
Therefore runtime tests exercise the completion edge; they do not pretend to
produce a naturally asynchronous timeout. The exact retail oracle, rather
than a test-only production hook, verifies the timeout/reissue topology.

## Independent oracle

`pc_port/tools/b54kj_6ad40_f0_wait_oracle.py` imports no production code. It
authenticates the executable and window hash, compares all eight words,
decodes both branch targets and the sole call, checks live result retention,
checks both boundary words, and closes the prefix arithmetic.

```text
  OK retail identity: exact executable and 8-word window hash
  OK complete retail comparison: 8/8 words
  OK retry topology: -1 -> 0x8006B064; positive -> 0x8006B098
  OK live poll: func_8006E7E8 -> s2; zero exit clears s0
  OK cut geometry: prior nop; next lui s1,%hi(D_800930E0)
  OK prefix arithmetic: 0x394 bytes / 229 words

B54K-J oracle: 6 check groups passed.
```

## Focused tests and gates

`B54KJ_6AD40_f0_completion_and_frontier` proves the canonical poll clears
`D_800B0CD8 & 0x01004000`, retains the F0 destination/provider state,
performs exactly the expected two `718D0` walks, and reaches only
`func_8006AD40_D_800930E0_cut`.

`B54KJ_6AD40_repeat_no_duplicate_work` reruns the complete prefix after CD
and run-control reset. Exactly four total image walks and two frontier hits
prove that the new wait does not replay `718D0` or `func_80030894`; the F0
completion state is clean after both invocations.

Normal suite:

```text
TEST B54KJ_6AD40_f0_completion_and_frontier... PASS
TEST B54KJ_6AD40_repeat_no_duplicate_work... PASS
Results: 948 run, 948 passed, 0 failed, 0 skipped
```

Fresh out-of-tree ASan/UBSan suite:

```text
Results: 948 run, 948 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

The retail executable remains exact. No scheduler destination, `m0360i`
special case, or persistence bit was added; scheduler provenance remains
`NEEDS_ARTIFACT` independently of this production-prefix advancement.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_800930E0_cut
SEMANTIC_IMPLEMENTATION=verified_through_0x8006B0D4
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
