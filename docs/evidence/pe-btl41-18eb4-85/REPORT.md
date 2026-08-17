# PE-BTL41 — type-3 0x85 / 18EB4 / 66B60

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018EB4` — 11 words `0x80018EB4..0x80018EE0`,
SHA-256 `1a18600c…9d43`. `D_800910A0[0x85]`.
`jal func_80066B60(lhu *arg0)`; v0=1.

Native: `pc_port/game/boot/func_80066B60_port.c` and
`func_80018EB4` in `func_80017018_port.c`.

`func_80066B60` — 30 words `0x80066B60..0x80066BD8`,
SHA-256 `862ff618…e45e`. Zero jal. v0=0.

```text
snap = (CFE8, CFEA, CFEC)
CFE8 = CFEA = CFEC = 0xFF
CFEE = 2
CFEF = 2
CFF6 = a0
CFF8 = 0
CFF0/CFF2/CFF4 = snap
```

Boot `6E9A0` already `jal 66B60(2)` after `5E588`.
That path is now the real leaf instead of the
bootstrap stub. Callers must have run `6A8D4`
first so `B0E38` is a guest packet; otherwise
`68E24` would load `pkt+0xC` at `0xC`.

Type-3 HIT imm is `0x1E`. Live type-0 first-visit
`0x0B` pose (`+0x28=0x100000`, `+0x30=0x5410000`)
still misses the type-3 `0x77` rects (X=16 is
outside `0x80A..0x994` and the later pairs).
Do not invent a region hit. `0x85` is ready
when an authentic hit occurs.

## Verify

```text
python3 pc_port/tools/pe_btl41_18eb4_oracle.py
PE_TEST_FILTER=BTL41 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
