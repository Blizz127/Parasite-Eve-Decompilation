# PE-BTL18 — 15DAC key 0x190 overlay table walk

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## Case `0x80016658`

`D_800101B0[0x190-100]`. Search body 34 words
`0x80016658..0x800166E0`. Match tail `0x800168A4` (shared
epilogue `0x800168F4`). Always v0=1.

```text
base  = *D_800B0E64          # overlay+0x18C / chunk2
word  = *(base + *(base+4) + 0x30)
count = word >> 22
off   = word & 0x003FFFFF
```

Walk `count` 12-byte rows at `base+off`. A row matches when
`(+3)&0x10` and `lhu(+0xA) == *arg1`. On match:

| dest | value |
|---|---|
| `D_800B0DB8` | matched key (u8) |
| `D_800B0DB9` | row+8 |
| `D_800B0DFC` | `base + (row+4 & 0x00FFFFFF)` |

No match / count 0: no stores.

## Live m0005i

chunk2 `hdr+0x30 = 0x00C2FD78` → count 3, table `+0x2FD78`.
Type-1 first real `0xEA` is key `0x190`, `*arg1=0x28`.
Row 0/1 lack bit 0x10. Row 2 matches:

```text
+3=0x10  +0xA=0x28  +8=0x17  +4=0x2E838
D_800B0DFC = overlay + 0x2E838
```

This is authentic published chunk2 data, not a host stream.

## Verify

```text
python3 pc_port/tools/pe_btl18_15dac_190_oracle.py
PE_TEST_FILTER=BTL18 ./pc_port/build/pe-native-tests
```
