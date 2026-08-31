# PE-B54K-B4 — `func_80030894` complete L7 packet group

Date: 2026-08-30

## Verdict

```text
IMPLEMENTED=0x80030F6C..0x800310A4 exclusive
WORDS=78
BYTES=0x138
WINDOW_SHA256=0b3149236b8f26a7cc5614dd4179039f1f33824a4c3618fef4cf64eca551659d
NEW_STRICT_FRONTIER=func_80030894_L7_cut
FIRST_EXCLUDED=0x800310A4 move s6,zero
PLANTED_STATE=NO
NEW_DEPENDENCIES=NONE
```

```text
0x80030894..0x800310A4 = 0x810 bytes = 516 words translated
0x800310A4..0x800314E4 = 0x440 bytes = 272 words remaining
516 + 272 = 788
```

## Retail unit

The predecessor is L6's real branch delay slot at `0x80030F68`. The first
included instruction materializes `D_8009E460`; the last included instruction
is L7's back-edge delay slot at `0x800310A0`. The first excluded instruction
resets the counter for the separate L8 loop.

Five static calls are all already native:

- `0x80030F88` and `0x80031058`: `func_800370DC` compound-sprite wrapper.
- `0x80030FD4` and `0x80030FE4`: `func_80077C64` SetSprt headers.
- `0x80031020`: `func_80077B64` SetPolyF3 header.

The sole branch is `0x8003109C -> 0x80031040`; the literal `sltiu ...,3`
proves three iterations. There is no other branch, indirect call, upload,
hardware action, callback, scheduler action, or alternate exit.

Instruction-exact scale chains produce bank strides 28, 32, 20, and 84 and
an item stride of 28. Bank 0 therefore constructs:

- a compound sprite at `0x8009E460`, UV bytes `E8 E0`, CLUT `0x7E13`,
  dimensions `24 x 24`;
- standalone sprites at `0x8009E498` and `0x8009E4A8`, RGB `E0 E0 E0` and
  `60 60 60`;
- a PolyF3 header at `0x8009E4D8`;
- three compound sprites at `0x8009E3B8 + index*28`, RGB `80 80 80`, CLUT
  `0x7E13`, dimensions `24 x 8`.

## Independent evidence

`pc_port/tools/b54kb4_30894_l7_oracle.py` imports no production source. It
checks the complete retail window, selected boundaries/scales/calls, exact
call and branch census, and a zero-backed packet model:

```text
OK window: 78 words / 0x138 bytes, SHA-256 exact
OK boundaries/scales/calls: 26 instruction-exact words
OK control flow: five native jal sites; one three-item back-edge
model_unique_written_bytes=87
model_write_map_sha256=db066c0811940babcc12af7bbc2d8759590686821f2cd05038bf2f8b49acdc87
OK independent model: compound/pair/PolyF3/three-item array
```

The full-RAM canary admits those same 87 sparse bytes and rejects bank 1,
packet gaps, and L8:

```text
Results: 936 run, 1 passed, 0 failed, 935 skipped
CANARY_RC=0
```

## Gates

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_l4_group... PASS
TEST B54KB2_30894_l5_group... PASS
TEST B54KB3_30894_l6_group... PASS
TEST B54KB4_30894_l7_group... PASS
Results: 936 run, 6 passed, 0 failed, 930 skipped
FOCUSED_RC=0

Results: 936 run, 936 passed, 0 failed, 0 skipped
FULL_RC=0

Results: 936 run, 936 passed, 0 failed, 0 skipped
SAN_RC=0
```

The frozen B54J full-function structural oracle remains 19/19 green.

Strict real-disc execution stops at the exact new frontier:

```text
STRICT_RC=1
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80030894_L7_cut
       called from: func_80030894
```

Normal real-disc execution reaches `func_80030894_L7_cut`, then the existing
`func_8006AD40_post30894_cut`, and exits zero. L7 only constructs guest packet
state, so VIS1 remains byte-identical to B54K-B3:

```text
LIVE_RC=0
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
nonzero_words=2063
nonzero_bounds=256,64..735,456
decode=PASS all 524288 words / 1572864 RGB bytes
VRAM_VS_B3=IDENTICAL
PPM_VS_B3=IDENTICAL
```

The legacy host framebuffer remains black. This rung makes no renderer,
field, gameplay, event, or battle claim and plants no state. L8 begins at
`0x800310A4`.
