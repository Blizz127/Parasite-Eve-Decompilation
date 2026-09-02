# PE-B54K-AS — post-movie path and the overlay title system

Status: **TRANSLATED AND MODEL-VERIFIED; memory-card poll and SE playback
remain explicit boundaries**.

Authority: `SLUS_006.62` SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`PE.IMG` SHA-1 `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`, overlay
`[0x03D2,0x0457)` at `0x8018EFF0`.  Span headers are listed in
`docs/evidence/pe-b54kaq-boot-path-map/REPORT.md` and re-authenticated by
`pc_port/tools/b54kas_title_oracle.py` (18 spans).

## Translated (all words executed on the PE_PORT_SKIP_FMV path)

- `func_80192CE8` `[0x80192E08,0x80192E3C)` + `[0x80192F60,0x80192F98)`:
  frame-loop entry reading `D_800B0DBA`/`D_800B0DBC`, flag-0x200 clear,
  return `$s3`.  The per-frame movie step `func_80192934` is the named cut
  `func_80192CE8_func_80192934_cut` (unreached with the bypass; the
  completion and pad-skip arms after it are not translated).
- `func_801909B4` `[0x80190D8C,0x80191410)`: the post-movie environment
  re-copy, four ClearImage calls, environment swap, dirty-rect LoadImage
  idiom (`PE_Overlay_UploadDirtyRect`), VSync/ResetGraph/PutDrawEnv/
  PutDispEnv, `func_8018F2F4`, the 8-node task pool
  (`0x801D11CC`, stride 0x34, `sltu` bound `+0x16C`), the kind-1 spawn with
  handler `0x8019319C`, and the complete title loop with its retire /
  pending-append list maintenance and attract counter (`+= s4`, bound 1000).
  The loop exit at `0x80191410` is the named cut
  `func_801909B4_80191410_cut`; the pad-skipped 60-frame wait is
  `func_801909B4_func_800425DC_skipwait_cut` (unreachable with the bypass).
- Overlay title system (`pc_port/game/boot/overlay_title_port.c`):
  `func_8018F2F4` (480x204 24-bit background LoadImage to both display
  halves), `func_8018F468` (union rect with the previous frame, packed
  24-bit region at `env+0x8080`, per-task draw dispatch, rect publication),
  `func_8018F7F0` (word-transparent lighten blit `max(dest, src*alpha>>8)`),
  `func_8018F958` (cursor blit incl. its transposed tail),
  `func_8018FBC0` / `func_8018FD04` / `func_8018FE1C` (task spawns, width
  `units*2/3` by the `0x55555556` idiom), `func_80190064` (title input via
  the `func_8005E038` remap: Start edge `0x800`, confirm `0x20`, up/down
  `0x1000`/`0x4000` with `D_801D11B8` edge state), and the eight task leaves
  `0x80192F98/2FE8/3084/30D8/316C/319C/31BC/3200`.  Task callback fields hold
  the retail guest addresses; the three dispatchers cut on unknown values.
- Executable helpers (`title_exe_helpers_port.c`): `func_8005E038`,
  `func_80042770`, `func_8003FFCC`, `func_800525EC`/`func_8005267C` (retail
  `D_800B0E08` guard kept; the SPU call `func_8006DF50` is recorded with its
  sequence id, not played).

## Explicit boundaries on the path

- `func_800425DC` (memory-card poll, 101 words, libcard state machine) is a
  `BOOTSTRAP_RET` boundary that records and continues.  With the two slot
  records of `D_800A0ED4` as `func_80042538` initialised them the title
  behaves as with no memory card.  **This is the skip_fmv strict frontier**
  (`func_800425DC` called from `func_801909B4`, first title frame).
- `func_8006DF50` sound effects, `func_80192934`, the loop exit path.

## Verification

Focused tests `B54KAS_title_pool_spawn`, `B54KAS_title_fade_sequence`,
`B54KAS_title_compositor_bytes` (hand-computed packed bytes and VRAM
placement `x*3/2`, `+0xF0` half), `B54KAS_title_input_start_and_confirm`.
The oracle's ByteModel rebuilds the steady-state title frame from the
overlay image data alone (background rows, kinds 1 and 2 at alpha 0x100,
packed-region alignment, LoadImage placement) and digests it:

```text
model title frame (320x240x24) SHA-256
033ae52bee471e5337961770da03797403341aaab30f4ddea41cbc81142f61ab
```

A real-disc run (`PE_PORT_SKIP_FMV=1 --max-frames 620 --vram-dump`) yields
both display buffers byte-identical to that digest.  Suite: 1002/1002.

```text
PRODUCTION_REACHABILITY(skip_fmv)=func_800425DC_from_func_801909B4
PRODUCTION_REACHABILITY(default)=blocked_at_func_80081314_func_8007F0C8_cut
TITLE_FRAME_MODEL_MATCH=EXACT_BOTH_BUFFERS
NEXT_RUNG=retail_reference_frame_comparison_and_input_state_change
```
