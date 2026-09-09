# PE-MV1d — func_8010C89C VLC frame decoder transcribed (2026-09-03)

## What landed

- `func_8010C89C` (overlay `0x8010C89C..0x8010CBF8`, ~225 words,
  disassembled from the authenticated 38-sector movie-module carve —
  PE.IMG `[0x039F,0x03C5)` -> `0x8010BCF8`, SHA-256 `d0a22a1a…0b40`) is
  now a full C transcription in
  `pc_port/game/boot/func_8010C89C_port.c`: a resumable VLC-style block
  decoder, pure guest-RAM plus one COP0 IEc touch (skipped with
  comment; no meaning under synchronous delivery). Arguments and static
  state are exactly as mapped in
  `docs/evidence/pe-mv1c-c89c-map/NOTE.md`:
  - `a0` = frame-stream cursor (`0` = resume from the 9-word saved
    image at `0x8011EB90`).
  - `a1` = output/arena base; tables sit at `a1+0x800` (`a2`) and
    `a1+0x10800` (`a3 = a2+0x10000`, recomputed on entry so the
    entry `a3` is dead).
  - `a2` = `[0x801D0DF8]`, loaded at the call site; C89C does
    `a2 += 0x800` first.
  - Static VLC state is 11 words at `0x8011EB8C..0x8011EBB4`
    (`0x8011`, not `0x8012`: the `lui` is `0x8012` but every `addiu`
    is negative), seeded by the module image.
  - Exits: pad (`CB84`, returns 0, the normal end-of-frame — pads
    `0xFE00` to the saved bound `[0x8011EBB4]`) and bound (`CBC8`,
    returns 1, saves the 9-word resume image). The caller ignores the
    return.

## Verification

- Oracle `pc_port/tools/pe_mv1d_c89c_oracle.py`:
  - module carve 38 sectors, SHA-256 exact;
  - decoder entry / a3-kill / pad-exit / bound-exit / EB8C-seed
    anchors;
  - call-site loads (`s3`/`a2`/`a1`), `jal C89C`, `jal 7C394`, EC
    stores;
  - an independent Python model of the decoder cross-checked against
    the C transcription over `MV1D_PAD / PAD3FF / BOUND / TABLE /
    MAIN / RESUME` vectors, including resume-equivalence (a fresh run
    to bound then an `a0 = 0` resume ≡ the uninterrupted run).
  - `PASS: mv1d anchors + model vectors`.
- Six focused tests `test_MV1D_c89c_*` in `pc_port/tests/test_native.c`
  assert the pad footprint, bound-exit save image, 0x3FF pad, table
  extract, main-loop marker (`0xFE00`), and resume paths.

## DAY2-158c follow-up (2026-09-09)

Live Disc1 on Bazzite with ungated C89C aborted at `PE_StoreU16` @
`0x80200000` — confirming this report's MV1d trap. Production
`func_801924F8` got_frame now **gates** C89C on Stage-1b readiness
(immediate pad or STR magic `0x80010160`). Non-ready streams take a named
Stage-1b boundary stop. **Do not clamp a1.**

Bazzite retest of `5223ecd` (gate + still-unconditional E0 promote): CLEAN
stop at `Stage1b_pad_terminated_frame` — MV1d fixed. DAY2-158d then gates
E0 promote on `B89F4` / fixture `PE_Port_ArmStreamPromote` so incomplete
bodies are not published; live last-chunk delivery remains the unlock for
real STR frames at `s1`.

## Production wiring (DAY2-158b/c/d — gated)

`func_801924F8` got_frame calls live C89C only when the stream is
Stage-1b-ready (pad or STR magic). Non-ready frames take
`Bootstrap_ReturnVoid("Stage1b_pad_terminated_frame", …)` +
`PE_PORT_STOP_UNRESOLVED_BOUNDARY`. E0 promotes only on last-chunk
`B89F4` or fixture surrogate arm. The decoder's output cursor is still
bounded only by VLC pad/terminator codes: a valid STR frame terminates
in-bounds. **Do not clamp a1.** Ungated decode of partial frames still
marches past `PE_StoreU16 @ 0x80200000` — that is why the gate exists.

## Verify block

```
cmake --build pc_port/build -j
python3 pc_port/tools/pe_mv1d_c89c_oracle.py      # PASS
PE_DISC1_BIN=<Disc1.bin> ./pc_port/build/pe-native-tests
#   Results: 1070 run, 1070 passed, 0 failed, 0 skipped
./pc_port/build/pe-native-tests
#   Results: 1070 run, 1052 passed, 1 failed, 17 skipped
#   (the one failure is the pre-existing B54KY env case: missing disc)
ASAN_OPTIONS=detect_leaks=0 PE_DISC1_BIN=<Disc1.bin> \
  ctest --test-dir pc_port/build-asan --output-on-failure
#   100% tests passed, 0 tests failed out of 2
./pc_port/build/parasite-eve-port --headless --strict-stubs \
  --disc-image <Disc1.bin>
#   FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
#          func_8010C89C  called from: func_801924F8
```

Not a matching leaf: no `src/` code added or claimed; leaf count
unchanged (560).

## Next rung

Stage 1b: the STR/MDEC frame-delivery pipeline that produces a real
decodable frame at `s1`, so the decoder can be wired live and the
frontier moves into the `func_80192CE8` media loop after `0x80192E08`.
