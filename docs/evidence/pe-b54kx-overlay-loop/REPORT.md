# PE-B54K-X — complete `func_80190660` 480-frame loop

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung completes overlay-local fade/display initializer `func_80190660`.
It executes all 480 retail loop iterations, returns normally, and advances the
production frontier to its caller's saved-bit branch at `0x80190D7C`. It adds
no scheduler, scene, destination, package, or story special case.

## Retail identity

```text
PE.IMG SHA-1               146c0ce7308bf9fdc2ba5a84230e198db0663f3b
function                    [0x80190660,0x801909B4)
size                        0x354 / 213 words
body SHA-256               fe2cc07abe6f50d8959ec5dbfe268d5ab6ac0f2fc91ffadc10f69a07e8a192ee
return                      0x801909AC jr ra / 0x801909B0 nop
exact-start caller          0x80190D74 jal 0x80190660 / nop delay
caller prefix               [0x801909B4,0x80190D7C), 242 words
caller-prefix SHA-256       449da8a3de6470a68186ffeadbddcfdc064b9e804ab4615774fdf29cb09d43d9
next instruction            0x80190D7C beqz saved_bit,0x80191120
```

The independent oracle checks all 213 function words, the exact caller, the
post-call branch, loop thresholds and back-edge, both display-flag stores, and
the canonical return.

## Loop contract

Retail intensity is represented literally:

```text
frames   0..31     frame * 4
frames  32..391    128
frames 392..423    (424 - frame) * 4
frames 424..479    0
```

Each iteration remains state-driven:

1. toggle/select one of the two environment pointers;
2. update the parity-local sprite RGB;
3. issue DR_MODE and SPRT through DrawPrim;
4. update the paired sprite RGB and perform explicit DrawSync;
5. execute the optional environment upload only when its signed width is
   positive;
6. VSync, ResetGraph(1), PutDrawEnv, and PutDispEnv;
7. increment frame and follow the authenticated `<480` back-edge.

Starting with canonical toggle zero gives exactly 240 selections of each
environment and ends on environment 0. The final epilogue hides display,
restores both environment display bytes to one, and returns zero.

Deterministic cardinalities for direct `func_80190660` execution are:

```text
SPRT rectangles             480
GP0 E1 commands              960 (DrawPrim + PutDrawEnv per frame)
each E2/E3/E4/E5/E6 command 480
DrawSync calls               1441
VSync calls                  480
PutDispEnv presentations     480
final display mask           0
```

The optional upload branch remains generic and state-gated. Canonical retail
state has width zero, so no upload is planted merely to exercise it.

## Tests and oracle

The two B54K-X focused contracts prove canonical 480-frame command/call
cardinality, final environment/cache state, normal return, and a nonzero
initial-toggle negative control that ends in the opposite phase without any
forced selection.

```text
focused B54K-X:    981 run, 2 passed, 0 failed, 979 skipped
normal full suite: 981 run, 981 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  981 run, 981 passed, 0 failed, 0 skipped
sanitizer errors:  0
```

Independent oracle output:

```text
  OK overlay: complete 213-word function and retail loop/return
  OK caller: 242-word prefix; exact call; 0x80190D7C next
  OK model: 480 intensities; 240/240 envs; 960 E1; 1441 syncs
  OK runtime: 2 focused contracts; post-initializer frontier

B54K-X overlay-loop oracle: PASS.
```

Production output:

```text
strict: func_801909B4_80190D7C_cut from func_801909B4
normal: [FB] vsyncs=486 drawsyncs=1444 presents=483 mask=0 main_iters=1
DMA:    calls=27 queries=26 services=26 captured=26 serviced=26
EXE:    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

```text
FUNC_80190660=COMPLETE_213_WORDS
OVERLAY_FADE_LOOP=480_FRAMES_STATE_DRIVEN
PRODUCTION_REACHABILITY=blocked_at_func_801909B4_80190D7C_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=audit_saved_bit_branch_at_80190D7C
```
