# PE-CH1 — BTL1 TRACE_CONTRACT integration

Handshake proof. Matching `src/` C was not added. `0x31` uses the
EXE-verified normal path of `func_80017BB4` for token `0xA80002C8`;
the unrelated `A9400048` special path is outside the named BTL1 cut.
`0x1A` runs translated `func_80070DD0(0,100)` and records the
19-word pre-call image (two indices + 17 table words). Consume cut is
`func_800299CC_consume_cut`.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
consume     0x800299CC..0x80029A0C exclusive (16 words)
persist     D_800A77F0 .. +0x800
0x89        D_8009D28C = 6 (matching func_80017FF0 guest store)
```

## Sequence (native test `BTL1_trace_mode6_consumed`)

| transition_state | How |
|---|---|
| `field` | start (`m0004i`) |
| `mailbox_3` / `mailbox_4` | real `func_80017764` payload 3 and 4 twin runs |
| `m0005i_enter` | real `func_80017BB4_btl1_cut`: destination token + load bit |
| `rng_selected` | real `func_80070DD0(0,100)`; `<19` → 49, else 50 |
| `slots_ready` | real `0x6F`/`0x70`/`0xB7`/`0x5A` (30220 tags 40–42/50–52) |
| `mode6_request` | `D_8009D28C = 6` |
| `mode6_consumed` | real consume cut: mode 6→0, `gp+0x10C = 6` |

`persist_hash` is SHA-256 of `D_800A77F0`..`+0x800` and must be
identical on the `0x89` row and the row before it.

`formation_id` carries the selected 49/50 plus 1332/1333/1334.
`player_state_hash` remains `partial`.

## Verify

```text
cmake --build pc_port/build -j2 --target pe-native-tests
./pc_port/build/pe-native-tests   # writes mailbox 3 and 4 BTL1 traces
python3 pc_port/tools/pe_ch1_btl1_trace_oracle.py
# focused: PE_TEST_FILTER=BTL1 ./pc_port/build/pe-native-tests
```

Native tests: full suite **647/647**. Oracle: consume 16/16 + mailbox 3
and mailbox 4 TRACE CSVs through `mode6_consumed`; opcode `0x31` handler
40/40 with BTL1 normal token path; persist_hash unchanged on each `0x89`
row.
