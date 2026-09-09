# PE-PROJ1 — mesh projection

2026-09-04 native continuation. Aya is not yet visible or playable.

Translated 3AC90..3AF14 from `asm/disc1/2A19C.s`. The mesh loop composes
the camera and each bone matrix, processes active vertex records in triples,
and writes XY/depth by vertex index into the retail buffers. A final short
record still processes a complete triple, as the original loop does.
35558 calls the existing orientation/clip/hierarchy paths, the still-partial
3A6A8 and new projection before 3AF14. Unpublished joint-storage fixtures
remain outside this resource-bearing continuation.

The sf=1/lm=0 RTPT coordinate helper uses integer Newton-Raphson reciprocal
and 44-bit MAC wrapping. Hardware reference:
https://psx-spx.consoledev.net/geometrytransformationenginegte/
(GTE coordinate commands and division inaccuracy sections).
It exposes only outputs consumed by the mesh loop: XY/SZ, MAC1-3 and IR1-3.
FLAG, IR0/MAC0 and persistent coordinate FIFO state are still unimplemented;
this is not a claim of a complete GTE RTPT implementation.

`PROJ1_mesh_batches_and_saturation` checks camera-plus-joint translation,
nonzero vertex start, padded triples, inactive bones, neighbor canaries,
zero/near depth, unit reciprocal and screen-coordinate saturation. It is a
constructed arithmetic fixture, not an independent hardware oracle.
Normal and ASan/UBSan CTest both pass 2/2, 1,086 native cases, with no
sanitizer diagnostics. Full suite logs: `/tmp/pe-proj-tests.log`, `/tmp/pe-proj-san-tests.log`.
Real disc 120-frame CLI reaches frame-limit with background; the newly
constructed/projected polygons are not yet submitted by 3AF14/3B144.

Next required work: model bounds/position path 3A6A8, lighting, polygon
submission and GPU rasterization, then animation/input/motion verification.
