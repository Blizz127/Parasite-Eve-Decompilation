# PE-BTL122 — D20C enemy body is 0x6F / 2F7D8

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## Spawn vs body

`125E0` / `35038(a1=0)` puts type 1 then type 6 on `D20C`.
Non-type-0 stores `*actor=0`. Empty `B0E70[type]` ORs
`+0x98` with `0xE0` and skips `1A680` / `362B8` / `3D050`.

m0005i type-1 `0x08` later constructs types 3, 0, 5, then
(after `0x09`/`0x05`) types 2 and 4. Type 0 is Aya
(`2F76C` → `B8A20`). Type 2 has `B0E70[2]`.

The only TEXT `jal 2F7D8` is `18954` (`0x6F`). That leaf
claims a 220-byte slot and stores `*actor=body`.

## Story gap

The allocator is already native. The playable m0005i route
reaches `0x55` → `0x89` → mode 7 without ticking type 1
through the fourth `0x08` (type 2) and type 2 through
`0x6F`. Do not plant a D20C enemy. Do not invent a body.

`35038` `+0x1AC!=0` tail (`1A680` / `362B8` / `3D050`)
is dest/clip init, not the `*actor` body store.

## Verify

```text
python3 pc_port/tools/pe_btl122_d20c_body_oracle.py
PE_TEST_FILTER=BTL122 ./pc_port/build/pe-native-tests
```
