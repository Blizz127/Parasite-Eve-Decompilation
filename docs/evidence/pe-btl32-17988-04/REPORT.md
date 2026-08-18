# PE-BTL32 — 0x04 / 17988 task-flag walk

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80017988` — 28 words `0x80017988..0x800179F8`,
SHA-256 `2ddb182b…5c0a`. `D_800910A0[0x04]`. Zero jal.
Always v0=1.

```text
for i in 0..2:
    task = *(D2F0 + 0xA0 + i*4)
    while task:
        if task != D300:
            task+8 |= 0x10
        task = *(task + 0x24)
```

Type-1 `0x1C` sends payload `0xFF` to dest type 0 /
id 0. `65400` deliver (3F3C4 @ `3F4E8`) requires
type-0 `+0x19C != 0` (set by first-visit `0x14`
code 2). The new task starts at `+0x19C` =
`base+0x618` (`0x1F`), so `local[4]=0xFF` hits
this `0x04`. Native `35558` walk cut does not yet
call `65400`; the leaf is the ROM body, not a
wired tick. Do not invent other mailbox payloads.

After the `0xFF` `0x04`, type-0 is `0x01` then
another mailbox test; `0xAA` is on that arm.

## Verify

```text
python3 pc_port/tools/pe_btl32_17988_oracle.py
PE_TEST_FILTER=BTL32 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
