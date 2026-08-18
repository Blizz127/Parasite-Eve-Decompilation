# PE-BTL123 — type-2 spawn is persist[0x4A] < 40

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Disc 1 chunk2 SHA-256 `01a64ba3…7e3b`. No matching `src/` C.

m0005i type-1 at chunk2 `+0x21678` after the first `0x02` yield:

```text
0x08 type 3
0x08 type 0
0x08 type 5
0x09 subop 0x0A  persist[0x4A] < 40 → local[0]
0x05 skip-if-false
0x08 type 2
```

`12850` jtbl[10] is signed `<`. New-game persist is 0, so the
fourth `0x08` runs. persist `>= 40` skips type 2. Do not poke
persist to force a spawn.

Type 2 still needs opcode `0x6F` / `2F7D8` for a D20C body
(BTL122). The playable route must tick type 1 through that
`0x02` and the four `0x08`s.

## Verify

```text
python3 pc_port/tools/pe_btl123_type2_gate_oracle.py
PE_TEST_FILTER=BTL123 ./pc_port/build/pe-native-tests
```
