# PE-BTL103 — 2B0E8 mode 2 → mode 9

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

This is **encounter-end / victory** aftermath, not player death.
Player death is mode 3 → `2B29C` → mode `-1` → dest `0xA9400048`.
Mode 2 ends at **mode 9** and does not write that dest.

## `2B0E8` phases (`D_8009CE74`)

| Phase | Behavior this cut |
|---|---|
| 0 | Wait Aya `+0x16==10` or `D1A0&0x800`; `+0x98\|=0x100`; phase++ |
| 1 | Wait `gp+0x534==1000`; phase++; `+0x98&=~0x100` |
| 2 | Wait `+0x0F==+0x1A`; `1A680(0x15)` or `0x18`; phase++ |
| 3 | `6D60C(0)==1` wait; else `295E4`, mode=9, `B0CD8&=~0x8000` |

Deferred: `703F4`, `4B70C` persist, `67CBC`.

## Mode-2 producer (not this cut)

`func_8002F300` stores mode 2 (`sw` @ `0x8002F570`).
Callers: `0x80029360` (Aya HP>0 after a walk) and
`0x8002B0C0`. Next rung.

## Verify

```text
python3 pc_port/tools/pe_btl103_2b0e8_oracle.py
PE_TEST_FILTER=BTL103 ./pc_port/build/pe-native-tests
```
