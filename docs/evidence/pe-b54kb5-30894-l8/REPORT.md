# PE-B54K-B5 — `func_80030894` complete L8 loop

Date: 2026-08-30

```text
IMPLEMENTED=0x800310A4..0x80031110 exclusive
WORDS=27
BYTES=0x6C
WINDOW_SHA256=9b87877a27ab26756ab9cfbdf9f6f5d4e31958975ca654fcdeaf87660f7b7ecb
NEW_STRICT_FRONTIER=func_80030894_L8_cut
FIRST_EXCLUDED=0x80031110 lui s0,%hi(D_8009E768)
PLANTED_STATE=NO
```

Prefix arithmetic:

```text
0x80030894..0x80031110 = 0x87C bytes = 543 words translated
0x80031110..0x800314E4 = 0x3D4 bytes = 245 words remaining
543 + 245 = 788
```

The predecessor at `0x800310A0` is L7's real branch delay slot. L8 begins
with its own counter reset and bank load. Its one call site at `0x800310E4`
targets native `func_800370DC`; the sole branch at `0x80031108` returns to
`0x800310CC`, with literal bound ten at `0x800310FC`. The delay slot at
`0x8003110C` is the final RGB store. `0x80031110` begins a distinct group.

Retail computes bank stride 280 and item stride 28. Bank 0 builds ten
compound sprites at `0x8009E500 + index*28`, each with draw mode
`0xE1000234`, combined length 6, sprite code `0x64`, zero tail link, and tail
RGB `80 80 80`. No upload or hardware action occurs.

Independent oracle:

```text
OK window: 27 words / 0x6c bytes, SHA-256 exact
OK control flow: one native jal site; one ten-item back-edge
model_unique_written_bytes=130
model_write_map_sha256=59cdcc608bd78cb548f46dc0ee86ad64b673429d21c353d7d03edfce65f9238c
OK independent model: ten compound sprites; next group untouched
```

Focused, sparse-canary, full, and sanitizer gates:

```text
Results: 937 run, 7 passed, 0 failed, 930 skipped
FOCUSED_RC=0
Results: 937 run, 1 passed, 0 failed, 936 skipped
CANARY_RC=0
Results: 937 run, 937 passed, 0 failed, 0 skipped
FULL_RC=0
Results: 937 run, 937 passed, 0 failed, 0 skipped
SAN_RC=0
```

Production proof:

```text
[STUB:BOOTSTRAP_RET] func_80030894_L8_cut (first invocation)
[STUB:BOOTSTRAP_RET] func_8006AD40_post30894_cut (first invocation)
[HOST] stop_reason=unresolved-boundary
LIVE_RC=0
STRICT_RC=1
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80030894_L8_cut
       called from: func_80030894
```

VIS1 remains byte-identical to B54K-B4:

```text
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
VRAM_VS_B4=IDENTICAL
PPM_VS_B4=IDENTICAL
```

The host framebuffer remains black. No renderer, gameplay, event, battle, or
planted-state claim is made. The next group begins at `0x80031110`.
