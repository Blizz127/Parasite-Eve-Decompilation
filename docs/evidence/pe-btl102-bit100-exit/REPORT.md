# PE-BTL102 — 3F3C4 bit 0x100 skips draw; dest-change; 1220C outer restart

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

This is **player-death teardown toward title/boot**, not
encounter victory and not field return to the encounter dest.

## 3F3C4 after `35558`

`andi 0x100` @ `0x8003F500` / `bnez 0x8003F5F4` skips
overlay/draw/`6A0E8`. It is **not** `jr $ra`.

When `6A25C` moved `D280` this tick (`D1C4` still the old
token), dest-change still runs: `74DC0` / `87024` /
`3DFC8(1)` / `696F0`, then `D1A0|=0x40` and `B0CD8|=2`.
Bit `0x100` is kept.

`3F624` pad-combo `6A25C` is not this cut.

## 1220C consume

`andi 0x100` @ `0x800124A4`. If clear, continue inner
(`0x80012294`). If set: `VSync(0)`, `SetDispMask(0)`,
`addiu $v1, 0xFEFF` (`~0x100` only), `j 0x8001224C`
(outer `6A5BC` / disc wait / `3E680` / `D280=0xA9400048`
title dispatch).

## Distinctions

| Path | Dest / mode |
|---|---|
| PLAYER DEATH | mode `-1`, dest `0xA9400048`, `B0CD8` bit `0x100` |
| ENEMY DEATH / VICTORY | not this machine |
| FIELD RETURN | not `0xA9400048` |

## Verify

```text
python3 pc_port/tools/pe_btl102_bit100_exit_oracle.py
PE_TEST_FILTER=BTL102 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL101 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL87 ./pc_port/build/pe-native-tests
```
