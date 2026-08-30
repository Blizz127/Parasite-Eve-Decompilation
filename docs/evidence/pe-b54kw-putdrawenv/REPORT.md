# PE-B54K-W — PutDrawEnv, GP0 environment, and first loop back-edge

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

> **SUPERSEDED EXECUTION FRONTIER:** B54K-X completes all 213 words of
> `func_80190660` and advances to caller PC `0x80190D7C`. This report remains
> authoritative for PutDrawEnv and GP0 E2h..E6h.

This rung translates the canonical state-bearing path of PsyQ PutDrawEnv
`func_80075424`, extends the terminal one-node GPU worker for drawing-
environment packets, and advances overlay-local `func_80190660` through
PutDispEnv and the first taken loop back-edge. No scene, package, scheduler,
destination token, or story state is introduced.

## Retail identity and function-hood

```text
disc1.candidate SHA-1       452fb033f2eaa4b18aa20a5bca60b8125af3a37b
func_80075424              [0x80075424,0x800754E4)
size                        0xC0 / 48 words
body SHA-256               31dd565c7af1c78d85eb4e50a6abaf8dde75857b6edfd4926f208bc858f7a852
preceding word              0x27BD0018 (real epilogue delay slot)
following word              0x27BDFFD8 (real next-function prologue)
return                      jr ra / addiu sp,sp,0x20
```

The executable has four direct exact-start callers:

```text
0x8005E7D8  0x80069FB8  0x8006A1AC  0x80070F40
```

The authenticated runtime-loaded overlay contributes a fifth direct caller at
`0x8019093C`. `docs/ai_context/sdk_map.md` independently string-proves the
symbol as PutDrawEnv. This is a callable function, not padding or a generated-
label inference.

## Wrapper and terminal linked-list contract

Retail builds a DR_ENV packet at `env+0x1C`, ORs its low 24-bit link field with
`0xFFFFFF`, calls `jtb[2](jtb[6], env+0x1C, 0x40, 0)`, and only after that
call returns copies exactly `0x5C` environment bytes to `D_8009575C`.

For the execution-proven environment `{clip 0,240,320,240; offset 0,240;
tpage 0xA; dtd=1; dfe=1; isbg=0}`, the complete node is:

```text
06FFFFFF  E303C000  E4077D3F  E5078000
E100060A  E2000000  E6000000
```

`func_80076B98` now accepts one terminal node (`low24=0xFFFFFF`, count 1..15).
The established four-word GP0(80h) node retains its synchronous MoveImage
path. All other represented words must be standalone GP0 E1h..E6h commands
or NOP. The entire shape is checked before GP1 changes, so malformed nodes
remain mutation-free. Multi-node ordering tables and isbg fill/tile opcodes
remain named packet boundaries; this rung does not claim a general GPU DMA
interpreter.

## Generic environment state

The hardware authority now stores full E2h..E6h command words and occurrence
counts. The existing GP0(64h) raster path consumes:

- E3/E4 inclusive drawing-area clipping;
- E5 signed 11-bit drawing offsets;
- E2 texture-window mask/offset transformation;
- E6 set-mask and check-mask behavior.

Each effect is conditional on the corresponding command having been observed,
preserving the earlier pre-PutDrawEnv physical-VRAM clip behavior. Unsupported
primitive depth/raw modes remain mutation-free.

## Overlay continuation

```text
PE.IMG SHA-1               146c0ce7308bf9fdc2ba5a84230e198db0663f3b
func_80190660              [0x80190660,0x801909B4) / 213 words
represented prefix         [0x80190660,0x80190960)
represented size           0x300 / 192 words
prefix SHA-256             6b548466e8e58277757897a9ab91c91b6e791a43a02c6e42c2a2c3adb92c6bc2
```

The nine newly represented words are:

```text
8019093C  jal PutDrawEnv
80190940  addiu frame,frame,1          (delay)
80190944  lui a0,0x801D
80190948  lw a0,0x11C4(a0)
8019094C  jal PutDispEnv
80190950  addiu a0,a0,0x5C             (delay)
80190954  slti v0,frame,480
80190958  bnez v0,0x801907C8
8019095C  andi a1,frame,1               (delay)
```

Frame one necessarily takes the branch. Native stops at the authenticated
loop target `0x801907C8` rather than duplicating or unrolling an unreviewed
480-frame body.

## Verification

Two focused tests independently cover the exact DR_ENV packet, dispatcher,
cache ordering, all environment-register counts, offset/clip/set-mask raster
effects, and a check-mask negative control.

```text
focused B54K-W:    979 run, 2 passed, 0 failed, 977 skipped
normal full suite: 979 run, 979 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  979 run, 979 passed, 0 failed, 0 skipped
sanitizer errors:  0
```

The independent oracle authenticates the complete executable body, boundaries,
caller census, all 192 overlay words, call/branch sequence, packet arithmetic,
source fences, focused tests, and real-disc frontier:

```text
  OK executable: 48 words, return/boundaries, four direct callers
  OK overlay: 192 words through PutDispEnv and loop delay slot
  OK model: terminal tag + E3/E4/E5/E1/E2/E6 packet
  OK runtime: 2 focused contracts; exact loop-reentry frontier

B54K-W PutDrawEnv oracle: PASS.
```

Production output:

```text
strict: first unresolved provider func_80190660_loop_reentry_cut
normal: [FB] vsyncs=7 drawsyncs=7 presents=4 mask=1 main_iters=1
DMA:    calls=27 queries=26 services=26 captured=26 serviced=26
```

```text
PUTDRAWENV_CANONICAL_DR_ENV=IMPLEMENTED
GP0_E2_E6_ENVIRONMENT=IMPLEMENTED
FUNC_80190660_PREFIX=192_WORDS_TRANSLATED
PRODUCTION_REACHABILITY=blocked_at_func_80190660_loop_reentry_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=translate_func_80190660_480_frame_loop
```
