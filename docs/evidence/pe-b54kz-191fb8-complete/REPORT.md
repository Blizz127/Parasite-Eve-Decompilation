# PE-B54K-Z — complete `func_80191FB8`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung completes the first overlay-local callee reached by B54K-Y and
advances `func_80192CE8` through the following direct call to
`func_801924F8(index)`. It adds no scheduler, destination, scene, story, or
persistent-state special case.

## Retail identity

```text
PE.IMG SHA-1       146c0ce7308bf9fdc2ba5a84230e198db0663f3b
function           [0x80191FB8,0x801922F4)
size               0x33C / 207 words
SHA-256            1064769b74b7d2c5dedec9cc9abfd7a2fd0f8c4db4cf0f90a3b897f7a0bc1aeb
return             0x801922EC jr ra / 0x801922F0 nop
direct calls       0x80192294 -> func_8007512C
                   0x801922D4 -> func_8007512C
caller continuation [0x80192CE8,0x80192E08), 72 words
next frontier      0x80192E00 jal func_801924F8 / signed-index delay slot
```

The function accepts exactly one or two pointers. Invalid counts, a null
member, or nonzero `D_800B0DBA` return zero before mutation. The caller's
`sp+0x10` list is transient and consumed synchronously, so native code passes
the pointer value by value rather than inventing a guest address for a host
stack slot.

For one source `b`, retail publishes:

```text
D_801D0DE8=b          D_801D0DEC=b+0x0FA00
D_801D0DFC=b+0x1F400  D_801D0DF8=b+0x3F400
D_801D0DF0=b+0x50400  D_801D0DF4=b+0x53100
```

For two sources, the first supplies `+0/+FA00/+1F400`; the second supplies
`+0/+11000/+13D00`. The function then copies two 20-byte DISP_ENV records
(`0x28` total) and two 92-byte DRAW_ENV records (`0xB8` total), sets
`D_800B0DBA=1`, `D_800B0DBE=0x98`, `D_800B0DBC=0`, and
`D_801D11B0=-1`.

The first retail MoveImage is always:

```text
source RECT     (320,0,192,256)
destination     (512,0)
```

Unless `D_800B0CD8 & 0x08000000`, it then performs:

```text
source RECT     (0,448,320,64)
destination     (512,256)
```

Both calls use the already-authenticated generic MoveImage dispatcher and
VRAM authority.

## Tests and oracle

The canonical B54K-Y real-disc contract now proves the one-source layout,
both MoveImage packets, environment copies, and next-call argument. Two new
B54K-Z contracts prove the two-source layout plus the skip-second-move bit,
and mutation-free count/null/busy guards.

```text
focused B54K-Z:    985 run, 2 passed, 0 failed, 983 skipped
normal full suite: 985 run, 985 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  985 run, 985 passed, 0 failed, 0 skipped
sanitizer errors:  0
```

```text
  OK retail: complete 207 words, normal return, two MoveImage calls
  OK model: one/two-source layouts; 0x28/0xB8 copies; move gate
  OK runtime: 2 focused contracts; func_801924F8 frontier

B54K-Z func_80191FB8 oracle: PASS.
```

Production remains deterministic:

```text
strict: func_801924F8 from func_80192CE8
normal: [FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0 main_iters=1
DMA:    calls=27 queries=26 services=26 captured=26 serviced=26
EXE:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

```text
FUNC_80191FB8=COMPLETE_207_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_from_func_80192CE8
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_func_801924F8
```
