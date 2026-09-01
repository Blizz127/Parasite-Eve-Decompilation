# func_80078FAC — volume attempt 8 — SDK-LIBRARY-COP2 (reclassified)

No C leaf is claimed; count remains 287.

Function hood is proven for file `[0x697AC,0x697B8)` / VRAM
`[0x80078FAC,0x80078FB8)`: direct `jal` at `0x80077F48`, retail body ends
`jr ra; nop`, preceding encoded word is the prior handwritten helper's return
nop at `0x80078FA8`, and the following word starts the distinct callable COP2
helper `func_80078FB8`.

Retail:

```text
80078fac: 48c4d800  ctc2 a0,$27
80078fb0: 03e00008  jr ra
80078fb4: 00000000  nop
```

Screens: frame 0; no calls, C-visible global writes, retained addresses,
indexing, loops, constants, or `$v0` liveness. The effect is the handwritten
GTE control-register transfer. Era `-O2 -G0` compiles the honest ordinary-C
boundary

```c
void func_80078FAC(unsigned int depth_cue_a) {
    (void)depth_cue_a;
}
```

to only:

```text
0: 03e00008  jr ra
4: 00000000  nop
```

The sole missing word is `48c4d800 ctc2 a0,$27`. This is the same proven
instruction-expressibility mechanism as attempt 7; a second phrasing cannot
create a COP2 side effect. Historical disposition was
`PARKED-HANDWRITTEN-COP2`. Candidate is in stash
`park volume func_80078FAC handwritten COP2 ctc2 unexpressible in C`; YAML,
build, and verifier were never changed.

Current disposition: `SKIP-SDK-LIBRARY-COP2`, as a member of the handwritten
libGTE/COP2 cluster screened in `../COP2_SDK_SCREEN.md`. This is redirectable
SDK support, not a PE1 game-code matching blocker.
