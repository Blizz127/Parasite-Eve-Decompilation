# PE-BTL109 — 28574 writes enemy body+0x10

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`299CC` @ `0x8002A470` jals `21054`. If that return is
`>0` and `D294!=0`, it jals `236E8` before `1D340`.
`23008` (from `2312C`) sets `D294` when `weapon+6==8`
or ammo remains.

`236E8` ORs body `0x2000` when `gp+0xE4==1` and the
`0x800BE830[D1D4]` slot holds the target actor.

`27D14` @ `0x80027F84` jals `28574` when
`body&0x6000==0x2000`. Default formula (record+0x4C
bits 8 and `0x10` clear):

```text
a1 = ((rec+0x1E)/5 + weapon+0) * ROM[0x800108E4][weapon+0x10&0xf] / 100
s0 = a1 - body+0x8C   # weapon+6==8
body+0x10 -= s0       # chip 1 if s0<=0
body = (body & ~0x6000) | 0x4000
```

This is Attack damage, not Aya `1F704` and not player death.

## Verify

```text
python3 pc_port/tools/pe_btl109_28574_oracle.py
PE_TEST_FILTER=BTL109 ./pc_port/build/pe-native-tests
```
