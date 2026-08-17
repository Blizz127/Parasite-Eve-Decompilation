# PE-BTL73 — Right from live `0x0B` pose hits type-3 `0x77`

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
m0005i chunk2 SHA-256 `01a64ba3…7e3b`.
No matching `src/` C.

Type-0 `0x0B` at `+0x5C` plants pose
`(+0x28,+0x2C,+0x30) = (0x100000, 0xFBA90000, 0x05410000)`.
`1CAB0` uses `(+0x28>>16, +0x30>>16) = (16, 1345)`.

Type-3 first `0x77` vertices (high halves):

```text
(0x0994, 0x04CD) (0x0994, 0x063D)
(0x080A, 0x063D) (0x080A, 0x04CD)
```

Z=1345 is already inside `0x4CD..0x63D`. X=16 is
left of `0x80A`. `710A4` Right is `BE9A2=0xFFDF` →
`D26C` bit 4. Type-0 `+0x20=0x10000`, `+0x26=0x1000`
gives `0x50000` per tick (5 units). 409 Rights put
X at `0x80D`, inside the rect. `1CAB0` then returns 1.

Command `0x15` becomes `0x16` after the first `710A4`;
row 22 still jalrs `710A4` on d-pad. After a hit the
script ORs `persist[0x18]` with 1 (not a skip gate)
and takes `0x85`.

`35C84` still has no wall clip. That is a labeled
cut limit, not a planted hit. Do not invent pad or
a geometry toggle.

## Verify

```text
python3 pc_port/tools/pe_btl73_77_hit_oracle.py
PE_TEST_FILTER=BTL73 ./pc_port/build/pe-native-tests
```
