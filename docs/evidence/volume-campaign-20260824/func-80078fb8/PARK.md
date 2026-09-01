# func_80078FB8 — volume attempt 9 — SDK-LIBRARY-COP2 (reclassified)

No C leaf is claimed; count remains 287.

Function hood is proven for file `[0x697B8,0x697C4)` / VRAM
`[0x80078FB8,0x80078FC4)`: direct `jal` at `0x80077F50`, canonical
`jr ra; nop` ending, preceding word is the prior helper's return nop, and the
following word `sll a0,a0,4` starts the next handwritten function.

Retail is:

```text
80078fb8: 48c4e000  ctc2 a0,$28
80078fbc: 03e00008  jr ra
80078fc0: 00000000  nop
```

Frame 0; no calls, C-visible globals, address retention, indexing, loops,
constants, coloring, or `$v0` liveness. Era `-O2 -G0` compiles the honest
ordinary-C boundary

```c
void func_80078FB8(unsigned int depth_cue_b) {
    (void)depth_cue_b;
}
```

to `jr ra; nop`; the sole missing word is `48c4e000 ctc2 a0,$28`.
Historical disposition was `PARKED-HANDWRITTEN-COP2`, the same proven no-intrinsic mechanism as attempts
7 and 8. Candidate is stashed as
`park volume func_80078FB8 handwritten COP2 ctc2 unexpressible in C`; no
integration file changed.

This is the third consecutive park, activating the campaign hard stop.

Current disposition: `SKIP-SDK-LIBRARY-COP2`, as a member of the handwritten
libGTE/COP2 cluster screened in `../COP2_SDK_SCREEN.md`. The hard stop remains
valid historical campaign control; this entry is no longer an active compiler
blocker for port-scope work.
