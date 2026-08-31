# PE-B54K-B3 — `func_80030894` complete mixed L6 packet group

Date: 2026-08-30

## Verdict

```text
IMPLEMENTED=0x80030D20..0x80030F6C exclusive
WORDS=147
BYTES=0x24C
WINDOW_SHA256=51fe2651b8b7df743b8fc277f79101757a0b0904c4fbee00d28dc4a30bf19125
NEW_STRICT_FRONTIER=func_80030894_L6_cut
FIRST_EXCLUDED=0x80030F6C lui s0,%hi(D_8009E460)
PLANTED_STATE=NO
NEW_DEPENDENCIES=NONE
```

Prefix arithmetic closes against the frozen 788-word function:

```text
0x80030894..0x80030F6C = 0x6D8 bytes = 438 words translated
0x80030F6C..0x800314E4 = 0x578 bytes = 350 words remaining
438 + 350 = 788
```

## Boundary, calls, and control flow

The predecessor is L5's real branch delay slot at `0x80030D1C`. The first
included instruction loads the outer bank at `0x80030D20`. The final included
instruction is L6's branch delay slot at `0x80030F68`; `0x80030F6C` begins a
new packet base/setup sequence.

The unit has nine static calls, all already native:

| PC | Target | Counted role |
| --- | --- | --- |
| `0x80030D3C`, `0x80030D4C` | `func_80077BC4` | two SetPolyG4 headers |
| `0x80030DF8`, `0x80030E5C`, `0x80030EBC` | `func_800370DC` | three compound sprites |
| `0x80030E0C`, `0x80030E70`, `0x80030ED0` | `func_80077B34` | shade-bit setters |
| `0x80030F34` | `func_80077C44` | three-iteration SetTile site |

There is one branch: `bnez` at `0x80030F64` back to `0x80030F2C`. Its
preceding `sltiu ...,3` proves exactly three tile iterations. There is no
other branch, `jalr`, hardware operation, upload, callback, scheduler action,
or alternate exit.

## Decoded packet state

All scale chains are instruction-exact:

- PolyG4 bank stride: `(bank*8+bank)<<3 = bank*72`; second packet is `+36`.
- Sprite bank stride: `(bank*8-bank)<<2 = bank*28`.
- Tile bank stride: `(bank*2+bank)<<4 = bank*48`; item stride is `16`.

The outer increment remains later, so this prefix executes bank 0.

The group constructs:

- PolyG4 packets at `0x800B0130` and `0x800B0154`, with the four exact RGB
  triples per packet preserved from retail.
- Shaded compound sprites at `0x8009E0B8`, `0x8009E2E8`, and `0x8009E320`.
  Each has draw mode `0xE1000234`, code `0x65`, RGB `80 80 80`, CLUT
  `0x7E13`, dimensions `8 x 4`, V byte `0xF4`, and U bytes `0x50/0x58/0x60`.
- Standalone tiles at `0x8009E358 + index*16`, index 0..2. Each receives
  the corresponding byte snapshotted from `0x8009CD90 + index` at function
  entry in all three RGB fields.

No semantic asset or UI names are assigned beyond proven packet types.

## Independent oracle and sparse canary

`pc_port/tools/b54kb3_30894_l6_oracle.py` imports no production source. It
checks the complete retail window, 32 selected boundary/scale/call words,
the exact call/branch census, and a separate zero-backed packet model using
canonical font bytes `11 A5 FE`.

```text
OK window: 147 words / 0x24c bytes, SHA-256 exact
OK boundaries/scales/calls: 32 instruction-exact words
OK control flow: nine native jal sites; one three-item back-edge
model_unique_written_bytes=106
model_write_map_sha256=0b043d826381f73ef82f05aa09c0eb6e7b274d546b5ba0c3b484afcadbef75d6
OK independent model: two PolyG4, three sprites, three tiles
```

The retained full-RAM canary admits exactly those 106 sparse bytes, not whole
packet ranges, bank 1, or the next group:

```text
Results: 935 run, 1 passed, 0 failed, 934 skipped
CANARY_RC=0
```

## Test gates

The focused B54K family retains A/B1/B2 and adds a B3 test covering both
PolyG4 color sets, all three shaded sprites, all three independently colored
tiles, the next-group negative, and the named frontier:

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_l4_group... PASS
TEST B54KB2_30894_l5_group... PASS
TEST B54KB3_30894_l6_group... PASS
Results: 935 run, 5 passed, 0 failed, 930 skipped
FOCUSED_RC=0
```

Full normal and fresh ASan/UBSan suites, both with the retail Disc 1 fixture:

```text
Results: 935 run, 935 passed, 0 failed, 0 skipped
FULL_RC=0

Results: 935 run, 935 passed, 0 failed, 0 skipped
SAN_RC=0
```

The frozen full-function B54J structural oracle remains 19/19 green.

## Production and visible-state gate

Strict real-disc execution stops at the exact new boundary:

```text
STRICT_RC=1
[DISC] boot executable loaded into guest RAM
[TRACE 0000] native_executable_start
[TRACE 0000] call_func_8001220C
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80030894_L6_cut
       called from: func_80030894
```

Normal execution reaches the inner and existing outer cuts and exits zero:

```text
[STUB:BOOTSTRAP_RET] func_80030894_L6_cut (first invocation)
[STUB:BOOTSTRAP_RET] func_8006AD40_post30894_cut (first invocation)
[HOST] stop_reason=unresolved-boundary
LIVE_RC=0
```

L6 only constructs guest packet state. It does not issue an upload, so the
read-only VIS1 artifacts remain byte-identical to B54K-B2:

```text
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
host_ppm_sha256=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
nonzero_words=2063
nonzero_bounds=256,64..735,456
decode=PASS all 524288 words / 1572864 RGB bytes
VRAM_VS_B2=IDENTICAL
PPM_VS_B2=IDENTICAL
SAN_LIVE_RC=0
RAW_NORMAL_SAN=IDENTICAL
PPM_NORMAL_SAN=IDENTICAL
```

The legacy 320x240 host framebuffer remains black. This rung makes no
renderer, field, gameplay, event, or battle claim. It plants no guest, VRAM,
destination, event, or battle state. The next unit begins at `0x80030F6C`.
