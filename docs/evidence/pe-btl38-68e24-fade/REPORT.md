# PE-BTL38 — 68E24 fade tick and 3F3C4 mailbox/fade cut

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80068E24` — 202 words `0x80068E24..0x8006914C`,
SHA-256 `bee869b9…4267`. Zero jal. v0=0.

```text
if (CFEE & 3) == 0: return
if (CFEE & 3) == 2:
    lerp CFE8/EA/EC → CFF0/F2/F4 by CFF8 / max(CFF6-1, 1)
    write CDDC slot +0x34/+0x35/+0x36
    CFF8++
    if CFF8 >= CFF6:
        CFEE = 0 if CFEE&4 else 1
else:
    copy CFE8/EA/EC into the slot
OT link through B0E38[CDDC] (host skips retail div-break)
```

Callers: `3F3C4` @ `0x8003F588` and `6E9A0` @ `0x8006EB4C`.
`3F3C4` reaches `68E24` when `B0CD8&0x100==0` and
`B0CD8&0x200==0`. `0xAA`'s `0x2000` bit is not that gate.
`1220C` stores `D1C4=D280` before the jal, so the `3F3E8`
equality holds on entry.

The native `3F3C4` cut is those two mailbox/fade sites plus
`65400` @ `3F4E8` and `35558` @ `3F4F0`. Other `3F3C4` jals
are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl38_68e24_oracle.py
PE_TEST_FILTER=BTL38 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
