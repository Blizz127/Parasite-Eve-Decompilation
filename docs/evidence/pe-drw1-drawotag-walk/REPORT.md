# PE-DRW1 — DrawOTag read-only chain walk + GP0(02h) FILL

Per `pc_port/docs/drawotag_decision.md`, the host walks the guest table
read-only; nothing here adapts ordering. Two discoveries set the shape:

- `func_80076B98` (18w) programs DMA2 (GP1 `0x04000002`, MADR = arg,
  BCR = 0, CHCR = `0x01000401`) — the actual packet walk is hardware.
  Its statics prove it (`D_80095854/58/5C/60` =
  `1F801814/1F8010A0/1F8010A4/1F8010A8` = GP1/D2 MADR/BCR/CHCR).
- `func_80076C34` at debug level 0 (production) never touches its ring:
  poll GPUSTAT bit 26, call the worker directly. The ring/queue/full
  paths are level-gated and stay cut.

Changes (native translations, no `src/`):

- `func_800753B4` (28w, new `game/boot/func_800753B4_port.c`): level
  gate, `jtb[2]` check, worker passes through to `76C34` exactly as
  retail passes it (the dispatcher validates the worker).
- `func_80076B98` (`game/boot/func_8007512C_port.c`): single terminal
  packet generalized to the chain walk. Head validates before the GP1
  write (prior cuts keep names/order); later nodes
  validate-then-submit progressively; size-0 OTC links send nothing;
  out-of-RAM next and zero tags end silently (retail provably survives
  the stub-tail jump every cleared-OT draw; a zero tag is never
  retail-written); node sizes ride the existing ≤15 bound; unknown
  shapes stay mutation-free cuts. No cycle guard — retail hangs on
  corrupt nonzero links identically.
- `func_800754E4` (`platform/pe_libgpu.c`): the deferred `76C34(76B98)`
  cut completed (same shape as `75424`), and the inverted level gate
  fixed (prints at level ≥ 2, 7506C pattern: record, stop, return —
  retail prints then continues; the draw below is the documented
  residual). `75424` gained the same `jtb` range guard.
- GP0(02h) FILL (`platform/pe_gpu.c/h`): command/coords/size states,
  15-bit VRAM fill clamped to VRAM (subset: no mask bit, area/offset
  ignored), `fill_*` telemetry. Required: every `isbg` DR_ENV ends in a
  FILL triple, so no field frame draws without it. The node validator
  walks structurally (env singles + FILL triples); truncated FILL stays
  a cut.

## Verify

```text
python3 pc_port/tools/pe_drw1_drawotag_oracle.py
PE_TEST_FILTER=DRW1 ./pc_port/build/pe-native-tests
```

6 focused tests: empty cleared-OT draw submits zero packets with the
table byte-identical; dispatch/jtb boundaries; two-node env chain with
telemetry; unknown-word cut (first node already submitted —
progressive, like hardware); FILL render pixels; level-2 print. The
BTL88 live-DRAWENV test now runs the production arm (draw fully
executes, FILL renders). Full normal suite: 1028 run / 1010 passed /
1 pre-existing environmental failure (`B54KY` missing disc path) / 17
skipped. ASan/UBSan: zero diagnostics.
