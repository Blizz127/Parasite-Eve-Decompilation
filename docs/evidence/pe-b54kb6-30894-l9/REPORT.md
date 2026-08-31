# PE-B54K-B6 — `func_80030894` complete L9 group

Date: 2026-08-30

```text
IMPLEMENTED=0x80031110..0x800311EC exclusive
WORDS=55
BYTES=0xDC
WINDOW_SHA256=7d64f3e10e16895bafd1b2fb71ed2757a5513072a35bd9772e04bd84ce56a739
NEW_STRICT_FRONTIER=func_80030894_L9_cut
FIRST_EXCLUDED=0x800311EC lui s0,%hi(D_8009E730)
PLANTED_STATE=NO
```

```text
0x80030894..0x800311EC = 0x958 bytes = 598 words translated
0x800311EC..0x800314E4 = 0x2F8 bytes = 190 words remaining
598 + 190 = 788
```

L9 starts after L8's real delay slot and closes after its own back-edge delay
slot. Calls at `0x8003112C` and `0x800311A4` both target native
`func_800370DC`. The sole branch `0x800311E4 -> 0x8003118C` has literal bound
four. The next instruction materializes a distinct packet base.

Bank 0 builds one compound sprite at `0x8009E768` (RGB `80 80 80`, UV
`58 EF`, CLUT `0x7E13`, dimensions `36 x 5`) and four compound sprites at
`0x8009E7A0 + index*28` (RGB `80 80 80`, same CLUT, dimensions `6 x 6`).

Independent oracle:

```text
OK window: 55 words / 0xdc bytes, SHA-256 exact
OK control flow: two native jal sites; one four-item back-edge
model_unique_written_bytes=97
model_write_map_sha256=4665016a28d54fdbf145211dc35d553678b9554c7d0a402c350a7f9025442636
OK independent model: one compound sprite plus four-item array
```

```text
Results: 938 run, 8 passed, 0 failed, 930 skipped
FOCUSED_RC=0
Results: 938 run, 1 passed, 0 failed, 937 skipped
CANARY_RC=0
Results: 938 run, 938 passed, 0 failed, 0 skipped
FULL_RC=0
Results: 938 run, 938 passed, 0 failed, 0 skipped
SAN_RC=0
```

Production reaches `func_80030894_L9_cut` and the existing outer cut in
normal mode (`LIVE_RC=0`); strict mode names L9 first (`STRICT_RC=1`). The
VRAM artifact is byte-identical to B54K-B5. This is packet construction only:
no upload, renderer, gameplay, event, battle, or planted state. The next unit
begins at `0x800311EC`.
