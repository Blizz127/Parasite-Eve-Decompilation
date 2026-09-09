# Opening field movie skip and staged map loading

The opening script now reaches M0010I and allocates Aya through the retail
script/actor path. This is not playable: the 120-frame display remains black,
Aya has not completed initialization, and input-controlled movement is unproven.

## Changes

* `func_80017018` dispatches opcode 35 (`80014E30`) under `--skip-movie` as
  an explicitly recorded host adaptation, returning retail's continuation value
  before loading the movie overlay or disabling display. With the flag off,
  this previously silent stall becomes an unresolved boundary retaining the
  instruction PC. Actual movie execution remains untranslated.
* Media waits at `80018FDC` and `80019060` now follow their retail status/active
  branches and retry by rewinding the instruction and setting task delay to one.
  `800190AC` is the retail return-one leaf.
* `6B4F8` consumes each previous chunk's texture entries before issuing the next
  synchronous host CD read. Retail issues asynchronously and processes those
  entries before polling completion (6B61C and 6B700). Host DrawSync drains
  guest-backed GPU sources before the next CD copy can reuse the staging arena.
  The former order overwrote chunk 1 with chunk 2 and dereferenced 81ABBFB0
  during the script-selected transition to M0010I.

No map token, Aya actor, player pose, or persistence bit is injected. The retail
M0431I type-one script selects token A8001048 (M0010I) after the skipped movie.
The real constructor publishes a type-zero actor with update identity 80035C84.

## Authority

Retail candidate EXE SHA-1: `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Bodies inspected against assembly and candidate executable:

| Address | Words | SHA-256 |
| --- | ---: | --- |
| 80014E30 | 106 | 3e11c6977f606fbb2b3ecf6cbce00a02ba1c6721419914bb01a2e791f84cd4a5 |
| 80018FDC | 29 | d4d716163bf322457e751789e6abb4e1a199fa2b63fd3bb3990927edb0065e0b |
| 80019060 | 19 | 673a3337ea50ab4ff3d545a5e2f1ec46fd6f78d000fa9be6562806b8632c74a2 |
| 800190AC | 2 | 5ce5ad86d452c4d2422bd63e15223d5d6b3dfb77f224a88c1f476e9fb34e359d |

Hashes establish inspected spans; they do not prove semantic equivalence.
Native translations/adaptations are not matching-C claims.

## Regression coverage

`SKIP2_script_movie_and_nop` checks skip continuation, default boundary/PC
retention, explicit adaptation logging, and no fabricated movie flags.
`SKIP2_media_wait_retry` checks inactive, active, and matching-status paths.
`SKIP2_opening_script_spawns_aya` executes 120 real-disc frames and checks the
script-selected M0010I destination, allocated Aya/type/update identity, movie
skip telemetry, and frame-limit completion. Before the staged-load ordering
fix this same execution aborted in the chunk-1 texture-header walk.

Normal and ASan/UBSan CTest pass 2/2: all 1,081 native cases and the
external library consumer pass, with no sanitizer diagnostics.

## Remaining work

At 120 frames Aya's task has stopped after opcode 5B (800182A0), currently
unwired in the VM. Matching source shows it clears camera flags C0 and returns
one. Upcoming Aya script opcodes include 2D, 9D, 75, 74, 7B; check their actual
providers and constructor dependencies before wiring. The model/animation
constructor suffix in 35038 is still omitted, including collision data used
by 9D. Do not infer initialized rendering or collision from actor allocation.

The prior black frame is consistent with the live subtractive-white fade packet
62FFFFFF / draw mode E1000440. Advancing the script to its legitimate fade
commands is still required; the screenshot is not visual proof of a scene.
