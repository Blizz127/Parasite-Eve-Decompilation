# PE-BTL100 — 1F078 mode 3; 2A7F8 → 2AA98 xor 2B29C

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.
`1F814` / `6DE80` / `2AA98` / `2B29C` / `53E6C` are
NONMATCHING_C native ports.

## 2A7F8

Compare chain, not a jump table. `2B0E8` is **mode 2**
(sole jal `@ 0x8002A84C`). Mode 3 `@ 0x8002A85C`:

```
record+0x4C bit 0x800  → 2AA98
53E6C(18) != 0         → 2AA98 (5409C if v0!=0)
else                   → 2B29C
```

Zero fixture (`D_8009D03C`/`D048`/`D050` = 0) makes
`53E6C(18)==0`, so player death takes `2B29C`.

## Case 0 (both machines)

`D_8009CE74` (gp+0x104) phase 0. Wait until Aya
`+0x0E==19` and `+0x0F==+0x16`. Else `+0x98 &= ~0x100`.
`1A680` `sw 0` at `+0x14` clears `+0x16`, so a 1-frame
clip (`resource+2==1`) completes on the first mode-3 tick.

- `2AA98` complete: `CE70=16`, `+0x98|=0x100`, phase++
- `2B29C` complete: walk `D20C` (skip Aya / `body+5==1`),
  `293F4(0)`, `CE70=70`, phase++, Aya `+0x98|=0x100`

`2B29C` case 5 stores mode `-1` and jals `6A25C` (not
this cut). HUD `77AC4` / `32B0C` / `27D14` / fade
phases stay deferred.

## 1F078

After a real `1F704` leaves HP<=0: mode 3, `4D4=0`,
`6DE80(0x46B)`, `1A680(19)`, `1F4B0`. Prefix HUD /
`21D4C` / `374E8` / `62F9C` / `67CBC` stay deferred.

## Verify

```text
python3 pc_port/tools/pe_btl100_mode3_oracle.py
python3 pc_port/tools/pe_btl99_6de80_oracle.py
PE_TEST_FILTER=BTL100 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL99 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL72 ./pc_port/build/pe-native-tests
```
