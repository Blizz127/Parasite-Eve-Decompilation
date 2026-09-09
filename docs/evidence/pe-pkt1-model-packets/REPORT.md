# PE-PKT1: model packets

Native continuation, 2026-09-04. Goal incomplete: Aya is still not visible
or controllable at the auditorium entrance.

## Implementation

`func_8003D050_packets` translates 3D0F0..3D5A4's packet writes:
GT4, GT3, G4 and G3 headers in alternating frame banks; semi-transparency
selected by geometry type; textured UV, CLUT and tpage payloads copied from
the model stream. Reserved packet bytes remain untouched for later pose/draw.
It returns the actual end cursor used by subsequent matrix allocation.
The caller's unused temporary UV pointer output is not exposed by this host
helper. Prefix and pointer calculations remain the existing translated cuts.

`func_8003D94C` translates 3D94C..3DBE4: page/CLUT relocation, half-page Y
wrapping and first-packet tpage 31 sentinel. The constructor follows
353B4..354B0 to select Aya's constants or another actor's packed field entry.
Full pose propagation/projection and 3AF14's draw branch remain deferred.

Authority: `asm/disc1/2CE38.s`, `asm/disc1/2E034.s` and
`asm/disc1/24240.s`, backed by retail EXE SHA1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

## Verification

`pc_port/tools/pe_pkt1_model_oracle.py` independently interprets the actual
retail instructions with delay slots. It executes the constructor slice
and relocation leaf without importing the C implementation. Test fixtures
include two polygons of each type, both transparency variants, odd parent
stream padding, byte-pattern UVs, disabled emission, four page/CLUT cases
(including signed inputs) and sentinel early return. C test
`PKT1_retail_packet_bytes` compares the complete 2-KiB output-buffer FNV
hashes to these retail executions and checks the packet-end cursor.

Normal and ASan/UBSan CTest both pass 2/2, 1,084/1,084 native cases,
with no sanitizer diagnostics. Sanitizer validation log:
`/tmp/pe-pkt-san-tests.log`; normal log: `/tmp/pe-pkt-tests.log`.
Real-disc CLI with --skip-movie runs 120 frames to frame-limit, with 120
presents, 123 vsyncs and 40,085 textured rectangles. These rectangles are
the background, not evidence of Aya geometry drawing.
Capture: `/tmp/pe-pkt-120.ppm`; run log: `/tmp/pe-pkt-120.log`.
