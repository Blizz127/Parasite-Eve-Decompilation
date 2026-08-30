# PE-B54K-Q — `func_801909B4` through the first `MoveImage`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

The real-disc overlay now enters translated native semantics instead of
stopping at the function symbol.  The first 149 words copy four retail
graphics environments, publish six arena pointers, execute three small state
owners plus `SetDispMask(0)`, and stop at the exact first unresolved call:
PsyQ `MoveImage` (`func_8007512C`) at retail `0x80190C08`.

No VRAM move, later overlay call, scheduler destination, `m0360i` case, or
persistence bit is synthesized.

## Retail identity and boundary

```text
executable SHA-1       452fb033f2eaa4b18aa20a5bca60b8125af3a37b
PE.IMG SHA-1           146c0ce7308bf9fdc2ba5a84230e198db0663f3b
full function          [0x801909B4,0x801918F8), 0xF44 / 977 words
implemented prefix     [0x801909B4,0x80190C08), 0x254 / 149 words
prefix SHA-256         e0fc1734a365940821d1ec4aa15ae64ef1bc9f6e043879421f47dae17900af06
boundary call          0x80190C08  jal func_8007512C
boundary delay slot    0x80190C0C  sh v0,0x1E(sp) (RECT.h = 256)
```

Function-hood remains proven by B54K-O: the complete 977-word body has 70
direct call sites to 32 targets and ends in its own normal `jr ra; nop` at
`0x801918F0`.  The following function starts with a real prologue at
`0x801918F8`.

The boundary captures the call as:

```text
target   0x8007512C
caller   func_801909B4
a0       retail stack-local RECT (host pointer never exposed)
a1       0x02C0
a2       0
RECT     { x=320, y=0, w=160, h=256 }
return   ignored by retail; native prefix retains s2=-1 only for unwinding
```

The full eight-byte RECT is copied into the value-only bootstrap argument
record.  The caller checks `PE_Port_ShouldStop()` immediately after the
translated prefix and therefore cannot consume that provisional return or
enter `func_8006E9A0`.

## Implemented semantics

The four copies are two contiguous source/destination groups:

| Retail source | Overlay destination | Bytes |
| --- | --- | ---: |
| `0x800BCDC8` | `0x801D1498` | `0x5C` |
| `0x800BCE24` | `0x801D14F4` | `0x5C` |
| `0x800BCE80` | `0x801D1550` | `0x14` |
| `0x800BCE94` | `0x801D1564` | `0x14` |

Together these are exactly `0xE0` source bytes copied to exactly `0xE0`
contiguous overlay bytes.  With the authenticated retail
`D_80011610=0x80120D00`, the six pointer publications are:

```text
0x801D11BC = 0x80120D00
0x801D11C0 = 0x8013CD80
D_800B0E50 = 0x80124D80
D_800B0E54 = 0x80140E00
D_800B0E38 = 0x80120D80
D_800B0E3C = 0x8013CE00
```

Three prerequisite translations are included:

- `func_8005E57C`, complete three-word `D_8009D120 = a0` setter;
- `func_8005C1EC`, complete execution-proven positive path: first positive
  call publishes `D_8009D030=1` and sets `D_800B0CD8|=0xC000`, repeats are
  inert; its zero path clears the scalar and honestly exposes
  `func_80042798` before the not-yet-executed post-call flag clear;
- `func_80042538`, complete 41-word reset: exact `0x830`-byte bzero, two
  `-1` sentinels, and thirteen scalar clears.

Their retail SHA-256 identities are respectively:

```text
5E57C  14884a4554cca56a4c1ea813ee236f4eb64fd0473ad54c89be8f8761efe999dd
5C1EC  4f06eb18ea6ffd322f864aa57de0945c0fefb2f3edb91996b45b72fd8cc5fb3c
42538  85b319440f7f0ed6464db12d742cc9ef03cb792c6bdefbeaab955de02d2e927c
```

## Tests and independent oracle

Four new tests plus the migrated strict-boundary test bring the suite from
958 to 962.  They cover direct helper behavior, the exact zero-path boundary,
all four copies, all six pointer calculations, every reset byte and sentinel,
MoveImage ABI/payload, whole-RAM byte canary, host display-mask isolation,
dirty repeat behavior, and strict termination.

```text
Results: 962 run, 5 passed, 0 failed, 957 skipped
Results: 962 run, 962 passed, 0 failed, 0 skipped
fresh ASan/UBSan: 962 run, 962 passed, 0 failed, 0 skipped
sanitizer diagnostics: 0
```

`pc_port/tools/b54kq_1909b4_prefix_oracle.py` imports no production code. It
authenticates the executable, PE.IMG, all 149 prefix words, both boundary
words, and all three helper bodies; independently models the copy and pointer
math; checks the caller stop guard; then runs both strict and normal real-disc
executions.

```text
OK retail prefix: 149 words then jal func_8007512C + h delay slot
OK prerequisites: exact 5E57C, 5C1EC, and 42538 retail bodies
OK source boundary: payload capture, stop, and caller guard
OK independent model: 0xE0 copied bytes and six arena pointers
OK real-disc runtime: overlay entered; MoveImage is next boundary
B54K-Q prefix oracle: PASS
```

Measured strict output:

```text
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_8007512C
       called from: func_801909B4
exit=1
```

Measured normal output stops before the caller consumes the prefix return:

```text
[STUB:BOOTSTRAP_RET] func_8007512C (first invocation)
[HOST] stop_reason=unresolved-boundary
exit=0
```

## Gates and disposition

```text
git diff --check:       PASS
disc1.candidate SHA-1:  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
FUNC_801909B4_PREFIX=149_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_8007512C_from_func_801909B4
NEXT_ARTIFACT_FREE_RUNG=generic_MoveImage_platform_authority
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
