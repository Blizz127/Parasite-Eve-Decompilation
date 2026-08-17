# PE-BTL13 — type0 vtable 35C84; 2F76C pointer stores

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

1A918 / E0060 / 371B0 / 125E0 are already ported. E0060 is
EXE-resident list-clear (27w inside tsize), not loaded overlay
and not M2. This rung does not reopen them.

## func_80035C84

96 words `0x80035C84..0x80035E04`, SHA-256 `ab904f90…da62`.
EXE rodata `D_800915DC[0]`. No TEXT `jal` sites (jalr from
`35558`). Same pose snapshot as `35E04`, then `361F4`. If
`D_8009D2E8` bit 0 is set, skip `3999C`. Else `jal 3999C`
(118w pad/button table, 0 jals; not this cut). Then the same
`+0x98` bit-1 motion integrate.

## func_8002F76C

27 words `0x8002F76C..0x8002F7D8`, SHA-256 `96c27c7b…e32c`.
Sole caller `35038@0x8003515C` on type 0.

```text
sw 0x800B8A20 → actor+0
sw 0x800B0CB0 → D_800B8A88
sw 0x8009D1B0 → D_800B8A8C
jal 5218C / 51980 / 51E64   # not this cut
```

## Verify

```text
python3 pc_port/tools/pe_btl13_35c84_oracle.py
PE_TEST_FILTER=BTL13 ./pc_port/build/pe-native-tests
```
