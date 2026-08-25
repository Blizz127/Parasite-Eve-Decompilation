# func_80079024 — volume attempt 7 — PARKED-HANDWRITTEN-COP2

No C leaf is claimed. Matching count remains 287.

## Function hood

- Span file `[0x69824,0x69830)`, VRAM `[0x80079024,0x80079030)`, 3 words.
- Body ends `jr ra; nop`.
- Direct retail `jal` callers include `0x8003E648`, `0x8003F1E8`,
  `0x8006685C`, `0x80068418`, and `0x80068BD0`.
- Outside words at `0x80079020` and `0x80079030` are encoded nops in the
  handwritten GTE-helper alignment area; neighboring function bodies are
  `func_80079004` ending at `0x80079018` and `func_80079034` beginning at
  `0x80079034`.

The canonical return and multiple exact-start direct calls prove this is a real
callable function, not one of the unreferenced zero-padding spans.

`FUNCTION_HOOD=PROVEN`.

## Retail semantics and screens

```text
80079024: 48c4d000  ctc2 a0,$26
80079028: 03e00008  jr ra
8007902c: 00000000  nop
```

This is the handwritten Psy-Q `SetGeomScreen` helper: it transfers argument
`$a0` to GTE control register 26 (`H`). Frame 0, no calls, no globals visible
to the C abstract machine, no address retention, indexing, loop, repeated
constant, or `$v0` liveness. The relevant pressure is not register coloring;
it is an architecture-specific side effect unavailable in ordinary C.

## Bounded candidate

The only honest ordinary-C boundary is:

```c
void func_80079024(unsigned int screen_distance) {
    (void)screen_distance;
}
```

Era `-O2 -G0` emits:

```text
00000000 <func_80079024>:
   0: 03e00008  jr ra
   4: 00000000  nop
```

It is 2 words versus retail's 3. The sole missing word is the semantic body,
`48c4d000 ctc2 a0,$26`. Repository searches found no sanctioned compiler
intrinsic for `ctc2`; existing documentation identifies this retail helper but
does not provide an ordinary-C spelling.

## Disposition

`PARKED-HANDWRITTEN-COP2`: matching requires inline assembly, a macro that
expands to assembly, or a new compiler intrinsic. All are outside this
campaign's allowed C subset; no second phrasing can change instruction-set
expressibility. YAML, build, and verifier remain unchanged. Candidate source is
preserved in stash `park volume func_80079024 handwritten COP2 ctc2 unexpressible in C`.

Unparking requires an explicitly sanctioned COP2 intrinsic policy/toolchain
feature, not a source-phrasing retry.
