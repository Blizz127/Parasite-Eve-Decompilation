# PE-B54K-AD — `func_801924F8` filename and search phase

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung advances the authenticated `func_801924F8` prefix from
`0x80192584` through its retail filename construction and CD-file search. It
stops before the first instruction that copies the returned `CdlLOC` into the
next state block.

## Retail identity and boundary

```text
complete function  [0x801924F8,0x80192934)
complete size      0x43C / 271 words
complete SHA-256   ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a
translated prefix [0x801924F8,0x80192614)
prefix size        0x11C / 71 words
prefix SHA-256     c5ac60f55bb880129bf6cb3363a84d1a3febec10fea7d81a78ea809658eb6c8c
next instruction  0x80192614 lui a2,0x801D
normal return      0x8019292C jr ra / 0x80192930 nop
exact-start caller 0x80192E00
```

Function-hood remains independently proven by the complete normal return,
exact-start caller, and real instructions on both outer boundaries.

## Proven filename/search contract

Retail zeroes a 32-byte stack buffer and selects one of two authenticated
overlay strings:

```text
(uint16_t)index < 21  -> "\\FMV1"
(uint16_t)index >= 21 -> "\\FMV2"
```

It calls `func_800719F4` twice: first with that directory, then with the
pointer in selected 20-byte record word 0. The executable proves that
`func_800719F4` is the three-instruction BIOS A(15h) `strcat` trampoline.
The resulting paths have the form `\FMV1\FMV001.STR;1` or
`\FMV2\FMV018.STR;1`; the destination never needs to exist as one literal
string.

The following loop is retained exactly at the semantic level:

1. initialize the candidate result to zero;
2. require `CdReady() == 1`;
3. require queue depth zero;
4. call `DsSearchFile(0x801D0DC4, filename)`;
5. retry while the result is zero or `-1`.

The new cut is `0x80192614`. Retail next copies the first four returned
`CdlFILE` bytes from `0x801D0DC4` to `0x801D0DDC`; that copy and all later
movie-state setup remain unimplemented.

## Independent controls

The positive retail-disc contract enters with index 1 after the complete
133-sector overlay load and proves:

```text
path       \FMV1\FMV001.STR;1
LBA        189742
size       42584064
CdlFILE id FMV001.STR;1
```

A synthetic ISO fixture independently exercises the threshold at index 21.
Its record points to the guest string `\FMV018.STR;1`, and only its FMV2
directory contains that file:

```text
path       \FMV2\FMV018.STR;1
LBA        42
size       2048
CdlFILE id FMV018.STR;1
```

The fixture is a negative/branch oracle only; it is not production state and
contains no scheduler, scene, or persistence values. Index 47 remains the
existing mutation-free out-of-range control.

## Verification

```text
B54K-AD independent oracle: PASS
B54K-AC regression oracle:  PASS
normal CTest:                2/2 PASS
native suite:                986/986
fresh ASan/UBSan CTest:      2/2 PASS
strict real-disc exit:       1
strict frontier:             func_801924F8_80192614_cut
retail EXE SHA-1:            452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No destination token, story bit, scheduler decision, FMV completion, or
post-search state is fabricated.

```text
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_71_WORDS
FILENAME_SELECTION=INDEX_THRESHOLD_21_PROVEN
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192614_cut
NEXT_ARTIFACT_FREE_RUNG=continue_func_801924F8_cdlfile_state_setup
```
