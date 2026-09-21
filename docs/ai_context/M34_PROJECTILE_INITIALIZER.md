# M34 projectile initializer and retained stack inputs

The full **255-word F434..F830** projectile initializer is translated as
`PE_M34BossProjectileInit(data, retained[6])` in `m34_boss_effect_port.c`.
It preserves the original rotation, signed halfword truncation, saturated
GTE matrix composition, velocity, position, flags and record padding.
The six original retained stack translations are explicit inputs.
[Native writer binding](M34_STACK_BINDING.md) now supplies them from the
sound, VM, command and drawing history. The connected replay executes its
five-projectile burst. Full Day2 and whole-route fidelity remain unproved.

Original M34 C2: Disc 1 LBA16597, 100 sectors, base8018EFE8, SHA-256
`0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a`.
F434's 255 words SHA-256:
`6908ebff535d8c51a2999ef3acc1dac48bfb3ec3e427991b5687f4f2e87280be`.
The retail and candidate executable SHA-1 remain
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Independent comparison

```
python3 pc_port/tools/pe_m34_projectile_init_oracle.py \
  --build-dir /tmp/pe-day2-release \
  --capture pc_port/build/day2-victory-evidence/pe-m34-core-connected.bin \
  --write-header
```

**252 original/native cases plus four captured-context variants PASS;
406 unique instruction PCs.** The interpreter executes the complete original
initializer and both original matrix helpers with no mocked callees. All
executed instructions and following words match the original EXE/overlay.
All RAM below `1FE000` is compared, with the original stack excluded. For
synthetic cases, the oracle additionally asserts every original write in
compared RAM is inside the 72-byte projectile record. Final GTE rotation,
translation, IR and MAC values are also compared; this is not a comparison
of every other GTE register/flag or full hardware timing.

Cases cross three input matrices (identity, quarter turn, extreme signed
coefficients), seven signed turns, three speeds and four six-word retained
stack patterns, including signed/wrapping boundaries. The captured-context
variants use the complete frame60,271 core capture, SHA-256
`fe8b6a9ba4b02dc3cbb61d35faf4287a06f950acd20d13e698dc653b691c106c`.
The six stack words are varied only in isolated differential calls, never
in a connected run. No default-zero assumption is made by the native function.

Generated `retail_m34_projectile_cases.h` contains compact input parameters,
original output records and GTE states. `test_m34_projectile.h` supplies the
original angle constant and required retail sine/cosine table words. The
full native suite passes **1,396/1,396**, zero failed/skipped. All **10
non-route CTest checks PASS in 62.44 seconds**. Release and Debug builds pass.
Logs: `/tmp/pe-m34-projectile-oracle.log`,
`/tmp/pe-m34-projectile-tests-build.log`,
`/tmp/pe-m34-projectile-debug-build.log`,
`/tmp/pe-m34-projectile-ctest.log`,
`/tmp/pe-m34-projectile-lasttest.log`.

## Original full-frame stack writer trace

A fresh ordinary-input cold boot stops deliberately at frame **60,270**,
one frame before effect creation: `stop=frame-limit`, story6C/arrival21,
roomA8003248. There is no new unresolved boundary in that capture.
Log `/tmp/pe-m34-before-effect.log`; complete 2 MB RAM capture
`pc_port/build/day2-victory-evidence/pe-m34-before-effect.bin`, SHA-256
`6e2e09c6310e60999a9ce81bea5f51e901a6d44ec3c66348694e7aba189dc584`.
This capture is never restored into the connected runtime.

```
python3 pc_port/tools/pe_m34_effect_stack_probe.py --frame \
  pc_port/build/day2-victory-evidence/pe-m34-before-effect.bin
```

The original **35558** frame executes to F434, tracing **6,674 unique PCs**
in the prefix. Source checks also cover the subsequent seven isolated F434
stack variants. This is a source-executed frame probe with supplied initial
stack/GTE state, not proof of the entire original boot or the retail machine's
incoming register/stack history. Output `/tmp/pe-m34-fullframe-stack.json`.

At F434 entry SP `801FEEE8`, data is `801863A0` and return address is
`800C2CE4`. The retained words have these latest writers within that frame:

| Offset from F434 entry SP | Value | Latest writer |
|---|---|---|
| -76 | 0 | No write in this frame |
| -72 | 0 | No write in this frame |
| -68 | 0 | No write in this frame |
| -36 | 80010690 | 8006916C, saved S1 (VM argument jump table) |
| -32 | 0 | 8018F0C8, fifth command argument |
| -28 | 0 | 8018F0D0, sixth command argument |

The earlier constructor/update-only probe missed these three same-frame
writers because its call graph was narrower. In particular, treating all
six words as zero would discard an observed original write. The first three
remain dependencies on earlier stack contents.

## Earlier history probes and limits

Two further ordinary-input cold boots produced complete 2 MB captures:

| Frame | Capture under `pc_port/build/day2-victory-evidence/` | SHA-256 |
|---|---|---|
| 60,190, command queued, mode 1 | `pe-m34-before-shot.bin` | 4a9d0ac7ccd6621247bc4bb98510855a4a995c8cd4117661d0950fad459edf48 |
| 60,197, two shots committed, mode 0 | `pe-m34-shot-committed.bin` | b2e4be2c7cc78f7ece78993b5528a80140748faa2dea1f96ab4bd382413fad41 |

Both deliberately stop at the frame limit; their historical optional route
milestones remain 50/57. Logs are `/tmp/pe-m34-before-shot.log` and
`/tmp/pe-m34-shot-committed.log`. Neither capture was restored into gameplay.

The probe's `--prefix-input-frame` mode executes original `3F404..3F4F8`,
including input, ordering-table setup, mailbox and field update. Only external
digital pad replies are supplied. From either capture this prefix reaches
an instruction-budget failure in graphics DMA polling: the call to VSync
has return address `800773E0`, with DMA control address `1F8010F0`.
The isolated interpreter does not model peripheral completion and advancing
hardware timers. This is a diagnostic limitation, not evidence of a native
gameplay defect or proof that menu selection alone caused the wait. No
replacement graphics/VSync callee or bypass was added.

Original `35558` alone returns normally for one call from both captures.
The longer pre-shot inner-field probe later exhausts its instruction budget.
The committed-shot inner-field probe completes 100 calls without reaching
F434, then fails its reachability assertion. Repeating the inner routine
omits the outer frame preparation, input and presentation history; it is
insufficient to establish the three remaining retained words. These failed
probes do not claim completed original-instruction comparisons.

Diagnostic logs: `/tmp/pe-m34-shot-prefix-diagnostic.log`,
`/tmp/pe-m34-committed-prefix-error.log`, and
`/tmp/pe-m34-committed-field-error.log`. All capture and probe processes are
terminal. The F830/FC54/FDE4 children have subsequently been translated and
compared in [M34_PROJECTILE_CHILDREN.md](M34_PROJECTILE_CHILDREN.md). Next work
is to recover the earlier stack writers with the relevant caller history.
Connected F434 dispatch remains unresolved.

The subsequent [firing-window study](M34_RETAINED_STACK_WRITERS.md) identifies
candidate writers for all six words after correcting the guest frame counter.
The earlier three-word gap is now a binding and preservation-proof task.
