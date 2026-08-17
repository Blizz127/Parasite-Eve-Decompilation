# PE-BTL75 — 67E1C camera-slot interpolate

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`67E1C` — 126 words `0x80067E1C..0x80068014`,
SHA-256 `99f90f7f…3d6f`. Zero jal. `68CE0` @
`68CF8`. `v0=0`.

Early-out: `(D1A0 & 0x104) != 0`. Live
`D1A0|=0x4000` still runs.

Container `*B1624`. Count `lhu +6`. Records at
`container + lw(+0x14)`, stride 56.

Bit 4: `acc = (lh(+0xC)<<8 | lbu(+0x20)) + lh(+0x1C)`;
`sh acc&0xFF → +0x20`; `sh mfhi((acc>>8)/lhu(+4)) → +0xC`.
Same pair on `+0xE/+0x22/+0x1E/+6`.

Bit 8: `acc = (lh(+8)<<8) + (lh(BCF8C)-lh(BD028))*lh(+0x1C)`;
`sh acc>>8 → +0xC`; `sh acc&0xFF → +0x20`. Same on
`BCF8E/BD02A/+0xA/+0x1E/+0xE/+0x22`.

If `BCF88&0x80`: clear that bit; `BCF90=BCF8C`,
`BCF92=BCF8E`. `66800` is the bit-0x80 setter.

Unpublished `B1624==0` skips the walk (host;
retail `6B4F8` publishes before `68CE0`).

`67A78` is the next `68CE0` tail (50w, jal
`67294` when rec bit 1 and `+0x24==BCFFD`).
Not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl75_67e1c_oracle.py
PE_TEST_FILTER=BTL75 ./pc_port/build/pe-native-tests
```
