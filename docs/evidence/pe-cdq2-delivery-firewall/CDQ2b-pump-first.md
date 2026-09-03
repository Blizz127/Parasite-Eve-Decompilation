# CDQ2b — lane-write addressing + reframe: pump first, -1 later (2026-09-03)

## Addressing breakthrough (retail 7FB44 bytes, `0x8007FB44..0x8007FBBC`)

Retail CD code does NOT use `$gp`-relative lane access. It
loads a global base once (`lui $a2,0x800A` /
`addiu $a2,$a2,0xB598` → `a2 = &D_8009B598`,
`a3 = a2 - 0x44 = &D_8009B554`) and stores with small offsets:

- `[B554] = [a2-0x44]`, `[B574] = [a2-0x24]`,
  `[B570] = [a3+0x1C]`, `[B578] = [a3+0x24]` (in the jal delay
  slot — the 0xB store, word-for-word vs the port).
- The earlier EXE scans (lui-then-store windows, lo16 ranges,
  `$gp`-relative) were structurally blind to this style, which
  is why they found only the B570 site.

`$gp` itself is dynamic (`0x800725B4: addiu $gp,$a2,0xCD70`
— from a boot argument, single `lui $gp` in the EXE), so
static gp-relative resolution was doomed twice over. Future
lane-writer hunts must follow pointer bases, not immediates.

## Reframe: pump first, -1 later

- The E0 give-up `while (7F72C() != -1)` may be error
  recovery that production never takes: NOTHING in
  {7ED58, 7FB44, 7C214, 7C564, 7F7E8, 7F88C} writes -1, and no
  -1 producer has been found.
- The real requirement is E0 SUCCESS: slots populated during
  the 2000 polls so `91B64 != 0 → got_frame` hits first try.
- Pump = invoke translated `func_8007C214` (publishes state 2)
  with its inputs set up (`[C0DC8]` records via the 7A214
  initializer, `[BE9E4]/[BE998]` indices). B0's 81314 streaming
  arm already installs 7C214 via DMA slot 3 — only the
  per-sector invocation (retail: interrupt-driven) is missing.
  The 7ED58 synchronous-reset precedent covers the
  interrupt-surrogate design.
- Open: invocation cadence during E0, and who runs the 7A214
  record setup on the B54KY path (80192CE8 directly — check
  prefix callers for the initializer).

## Next concrete steps

1. Trace B0's 81314 streaming arm on B54KY state: confirm
   callbacks installed, record `[C0DC8]/[BE9E4]/[BE998]`.
2. Prototype the pump (7C214 per E0 poll batch) behind the
   existing MV1B plant design; observe got_frame.
3. Only then: BD4C reland → C89C → movie.
