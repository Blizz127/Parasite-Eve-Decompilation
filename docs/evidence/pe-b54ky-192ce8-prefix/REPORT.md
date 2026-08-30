# PE-B54K-Y — saved-bit branch and `func_80192CE8` read prefix

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung resolves `func_801909B4`'s saved-bit branch at `0x80190D7C` and
translates the authenticated prefix of overlay-local `func_80192CE8` through
its first unresolved overlay-local call. It adds no scheduler, package,
destination, scene-name, story, or persistent-bit special case.

## Retail identity and control flow

```text
PE.IMG SHA-1               146c0ce7308bf9fdc2ba5a84230e198db0663f3b
func_80192CE8              [0x80192CE8,0x80192F98)
complete size              0x2B0 / 172 words
complete SHA-256           ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7
translated prefix          [0x80192CE8,0x80192DFC)
prefix size                0x114 / 69 words
prefix SHA-256             0670dc9a913589495f623812b226d1ac665ca7984a6cbbe15efc7c200c038855
next instruction           0x80192DFC sll a0,s1,16
complete return            0x80192F90 jr ra / 0x80192F94 nop
```

The caller branch is unambiguous:

```text
80190D7C  128000E8  beqz s4,0x80191120
80190D80  00000000  nop
80190D84  0C064B3A  jal 0x80192CE8
80190D88  24040001  li a0,1
```

`s4` is the `D_800B0DCD & 1` value retained from `0x80190A90`. Disc 1's
authenticated mount path sets that bit when `PE.IMG` is present, so canonical
production takes `func_80192CE8(1)`. The zero arm is preserved separately as
the exact structural boundary `func_801909B4_80191120_cut`; it is not folded
into the positive path.

## Translated prefix

Retail performs, in order:

1. `D_800B0CD8 |= 0x200`;
2. write byte one to `0x801D0E04 + index*20`;
3. `SetDispMask(0)`, `DrawSync(0)`, `ResetGraph(1)`;
4. issue the first table-selected read, retrying only an issue result of `-1`;
5. poll until zero, restarting the issue only on poll result `-1`;
6. `EnterCriticalSection`, `FlushCache`, `ExitCriticalSection`;
7. place a computed guest pointer in `sp+0x10` and call
   `func_80191FB8(1, sp+0x10)`.

The native port preserves the retry/poll CFG and existing generic disc/GPU/
critical-section providers. A native stack address has no stable guest
identity, so the unresolved-call record stores `a1=0` and snapshots the one
stack word by value. No host pointer is written to guest RAM or retained.

The first read derives entirely from authenticated executable data:

```text
D_8009315E            0x039F
D_80093160            0x03C5
D_80093162            0x03C9
D_800B0DD8            1013 (PE.IMG LBA, mounted state)
D_8001160C            0x8010BCF8 (retail destination pointer)
issue LBA             1013 + 0x039F = 1940
sector count          0x03C5 - 0x039F = 38
byte count            38 * 0x800 = 0x13000
payload SHA-256       d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5ba8d0b40
stack word            0x80120D00 + ((0x03C9-0x03C5)<<11)
                      = 0x80122D00
```

The complete function after the cut calls `func_801924F8` and enters a larger
media/event loop before clearing `0x200` in its real epilogue. None of that
remainder is approximated. In particular, the prefix correctly leaves
`0x200` set when the unresolved `func_80191FB8` boundary stops execution.

## Focused contracts and negative control

The real-disc positive contract installs the retail table and pointers, runs
`func_80192CE8(1)`, confirms the indexed byte and busy bit, checks the first
two loaded payload words (`7`, `"MDEC"`), and verifies the exact
`func_80191FB8` target/caller/argument/value snapshot.

The saved-bit-zero contract executes the complete caller prefix and 480-frame
initializer without an active disc. It reaches only `0x80191120`, leaves the
indexed record and CD completion sentinel untouched, and never sets `0x200`.

```text
focused B54K-Y:    983 run, 2 passed, 0 failed, 981 skipped
normal full suite: 983 run, 983 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  983 run, 983 passed, 0 failed, 0 skipped
sanitizer errors:  0
```

Independent oracle:

```text
  OK overlay: 172-word function; exact 69-word prefix and first callee
  OK caller: zero -> 0x80191120; nonzero -> func_80192CE8(1)
  OK data: PE.IMG+0x39F, 38 sectors; stack word 0x80122D00
  OK runtime: real-disc read; negative arm; strict frontier

B54K-Y 80192CE8-prefix oracle: PASS.
```

Production output:

```text
strict: func_80191FB8 from func_80192CE8
normal: [FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0 main_iters=1
DMA:    calls=27 queries=26 services=26 captured=26 serviced=26
EXE:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

```text
FUNC_80192CE8_PREFIX=69_WORDS_TRANSLATED
SAVED_BIT_POSITIVE_ARM=RETAIL_TABLE_DRIVEN
SAVED_BIT_ZERO_ARM=EXACT_80191120_STRUCTURAL_CUT
PRODUCTION_REACHABILITY=blocked_at_func_80191FB8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_80191FB8
```

Postscript: B54K-Z subsequently completed `func_80191FB8`; the current
frontier is `func_801924F8` from `func_80192CE8`, with 985/985 tests. The raw
B54K-Y frontier above is retained as the state measured by this rung.
