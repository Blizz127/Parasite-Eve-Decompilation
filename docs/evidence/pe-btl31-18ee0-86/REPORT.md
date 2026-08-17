# PE-BTL31 — type-1 0x86 / 18EE0 / 66C7C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018EE0` — 11 words `0x80018EE0..0x80018F0C`,
SHA-256 `d042e2ad…9fec`. `D_800910A0[0x86]`.
`jal func_80066C7C(lhu *arg0)`; v0=1.

`func_80066C7C` — 27 words `0x80066C7C..0x80066CE8`,
SHA-256 `ce26b53b…ef0`. Zero jal. v0=0.

```text
snap = (CFE8, CFEA, CFEC)
CFEE = 6
CFE8 = CFEA = CFEC = 0
CFF6 = a0
CFF8 = 0
CFF0/CFF2/CFF4 = snap
```

Live type-1 after the type-2/type-4 spawns:
`persist[0x4A]==39` is false (live 0), so `0x05`
skips the `0x00` goto and **takes** `0x86` with
imm `0x1E`. The persist==39 arm skips `0x86`.
Do not invert that polarity. `0xAA` on type-0 is
the opposite shape and stays skipped.

After `0x86` the stream is already-ported `0x1C`
(mailbox `0xFF`) then `0x02`. `0x04` remains
mailbox-gated. `0x85`/`66B60` is the type-3 HIT
twin; do not invent a hit.

## Verify

```text
python3 pc_port/tools/pe_btl31_18ee0_oracle.py
PE_TEST_FILTER=BTL31 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
