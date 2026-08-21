# PE-CH2 — Carnegie prefix camera leaves

Three native translations, each independently verified against the
SHA-1-exact USA Disc 1 executable:

```text
exe_sha1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
65954     0x80065954..0x8006599C exclusive, 18/18 words
659C8     0x800659C8..0x800659F8 exclusive, 12/12 words
66800     0x80066800..0x8006698C exclusive, 99/99 words
```

No matching `src/` C was added: these functions remain inside the
unmaterialized monolithic `55430.s` split in this checkout.

## Proven behavior

- `func_80065954(index, enabled)`: camera slot =
  `*D_800B1624 + *(container+0x10) + index*16`; OR byte 0 with 6 when
  enabled, otherwise AND with `0xF9`.
- `func_800659C8(index, value)`: same slot; store halfword
  `(value >> 8)` at `slot+8`.
- `func_80066800(index)`: view =
  `*D_800B1624 + *(container+0x1C) + index*52`; publish H through
  `*D_800BCFA8` and `func_80079024`, copy nine rotation halfwords and
  three translation words through `*D_800BCFA4`, store the byte index
  at `D_800BCFFD`, and set `D_800BCF88 |= 0x80`.

Every production store above is present as `sb`, `sh`, or `sw` in the
recorded EXE windows.

## Route boundary

The opcode census disproves a single-scene m0004i sequence:

- m0003i module 0 runs `0x7B(0,0x4000)`, `0x7B(1,0x4000)`,
  `0x75(0,1)`, `0x75(1,1)` at PCs `0x4D0..0x500`.
- m0372i and m0004i apply `0x82(1)`.
- m0004i then reaches m0005i through the existing verified
  `0x31 0xA80002C8` path.

The native trace uses a synthetic record-1 fixture because retail
m0004i package bytes are not committed. It verifies copy semantics and
the real static opcode sequence, not projection pixels or framing.

## Verification

```text
cmake --build pc_port/build -j2 --target pe-native-tests
PE_TEST_FILTER=CH2 ./pc_port/build/pe-native-tests
python3 pc_port/tools/pe_ch2_prefix_camera_trace_oracle.py
./pc_port/build/pe-native-tests
```

The integration oracle delegates to all three leaf oracles and checks
the committed census before validating the generated CSV against
`TRACE_CONTRACT.md`.

Verified result: full native suite **651/651**, all three leaf oracles,
and the route-trace oracle pass.

## Excluded

No ATB, mode 7, enemy AI, field VM, `func_800677FC` projection chain,
screen-pixel claim, auto-apply rule, or invented camera behavior.
