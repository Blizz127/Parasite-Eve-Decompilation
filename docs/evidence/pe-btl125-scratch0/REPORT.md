# PE-BTL125 — type-6 waits while scratch[0]&4 is clear

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. Do not poke `D_800B6A80`.

New-game type-0 `0x09` subop `0x0B` (`persist[0x4A]==39`) is
false, so `0x05` skips to `+0x608` and never sends
`0x1C(2,0,0xB)`. Type 2 parks on `0x20` at `+0xA0`.

Type-6 `+0x190`:

```text
0x09 subop 3  cond[0] = scratch[0] & 4     # kind4 D_800B6A80[0]
0x09 subop 7  cond[1] = (cond[0] == 0)
0x05          if cond[1]==0 goto +0x1E8    # 0x12 fork
0x02 / 0x00   yield and loop +0x190
```

Clear bit waits. Set bit takes `+0x1E8` then `0x89` /
`0x1C(2,0,0x7D)` (the new-game type-2 wake).

EXE `lui/addiu 0x6A80` sites are only `1266C` zero, `17018`
kind-4 decode, and `34F10` zero. dest-ready `1266C` zeros 64
words at `D_800B6A80`. First m0005i `0x2A[0,2]` is type-6
`+0x1850`, after unported `0xAE` — not the first-visit
producer. Do not force the bit.

## Verify

```text
python3 pc_port/tools/pe_btl125_scratch0_oracle.py
PE_TEST_FILTER=BTL125 ./pc_port/build/pe-native-tests
```
