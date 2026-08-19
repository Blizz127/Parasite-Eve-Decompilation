# PE-BTL127 — 6BE4C dest-enter CE2/CE3 gate

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`6BE4C` — 32 words `0x8006BE4C..0x8006BECC`,
SHA-256 `fb4091da…306b`. Sole TEXT jal `3F074@0x8003F204`
after `1266C` and before `6BECC`. v0=0.

```text
if CE2 not in [10,15): return
if CE2 != CE3: overlay |= 0x200000
if (CE2-10)>>1 != ((CE3-10 srl 31)+CE3)>>1:
    overlay+0x0E |= 4
```

Writes `B0CD8` / `B0CE6`. Not `D_800B6A80`. dest-ready now
jals it after `1266C`. Live m0005i CE2=10 applies the
retail CE2/CE3 compare; scratch[0] stays clear.

## Verify

```text
python3 pc_port/tools/pe_btl127_6be4c_oracle.py
PE_TEST_FILTER=BTL127 ./pc_port/build/pe-native-tests
```
