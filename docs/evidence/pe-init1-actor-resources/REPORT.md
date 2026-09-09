# PE-INIT1: actor resource and clip initialization

2026-09-04. Native port continuation; no matching src/ or executable edits.
Aya is **not yet playable**. With `--skip-movie`, the natural opening script
loads M0010I and clears its fade, showing the limousine street background.
The 120-frame capture `/tmp/pe-init.png` was inspected; Aya is not visible.
A 600-frame headless run reaches frame-limit (600 presents, 603 vsyncs).

## Changes and authority

- Resource-bearing 35038 constructor continuation: animation command choice,
  allocation, instance setup through existing 3D050 cuts, rotation/fade
  initialization and 3D834 clip binding. Authority: 3529C..35520 in
  `asm/disc1/24240.s`; 3D050 initialization in `asm/disc1/2CE38.s`.
- VM 5B/2D/9D/75/74/7B follows matched leaves and retail dispatch. 9D
  writes model header scale after resource initialization.
- 39B74, 39D24 and 39ED4 clip metadata, translations, angle tracks and
  in-range joint modifiers: `asm/disc1/2A19C.s`.
- 79754 alternate Euler matrix order: `asm/disc1/684EC.s`.
  `pc_port/tools/pe_init1_rotation_oracle.py` independently executes the
  original MIPS instructions from the SHA1-checked retail executable on
  seven synthetic trig-table fixtures. C tests compare all nine matrix
  entries to those outputs and check preserved padding/translations.
- Disc-backed boot fixtures adopt retail globals after EXE load to match
  application startup; previous fixture arena addresses were eight bytes off.

## Limits

35038/3D050 remains partial: packet constructors and UV work are omitted.
Full pose propagation and 3AF14's 3B144 draw branch are deferred. No input
movement, doorway crossing, collision or auditorium entrance claim.
39B74 uses call-local angle rows; an override outside decoded joint count
records `func_80039B74_scratch_index` instead of inventing retained scratch
contents. The rotation oracle verifies only its seven fixtures; clip
fixtures exercise both encodings and one valid joint override.

## Validation

Normal and ASan/UBSan CTest both pass 2/2 after the final constructor
initialization change, with 1,083/1,083 native cases and no sanitizer
diagnostics. Logs: `/tmp/pe-init-full.log` and
`/tmp/pe-init-asan-full.log`. Independent rotation oracle passes seven cases.
The strengthened 120-frame opening test checks script-created Aya, valid
model allocation, clip binding, pose values and nonblack framebuffer output.
