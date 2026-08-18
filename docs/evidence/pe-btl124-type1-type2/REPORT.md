# PE-BTL124 — dest-ready m0005i type-1 ticks type-2

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Disc 1 m0005i chunk2 SHA-256 `01a64ba3…7e3b`. No matching `src/` C.

Dest-ready `6B35C+6B4F8+6BECC+6C5BC+125E0` on token `0xA80002C8`
creates type 1 then type 6. Type-1 PC is dest2+`0x21678`
(`0x0000C0EA`). `65400` then `35558` ticks that script through
the first `0x02` and the four `0x08`s. persist `[0x4A]==0`
takes the type-2 spawn (BTL123). `35038` zeros `*actor`.

m0005i dest header byte+1 is **CE2=10**, not 14. dest-ready
Writer B therefore binds PE.IMG `[288,316)`. Type-0 command
`0x15` in that bank has `byte+2==1`. `D_800930D8` is EXE data
(same class as the dest charset / `93378` table). `1A680` has
no null check (`lbu 2($a0)` at `0x8001A6F0`).

`35558` `0x80035C2C` jals `360B4`, which `and`s actor `+0x98`
with `0xFF7FFFFF` so the next frame's `1A4AC` walk is not
skipped. `0x80035C34` then `and`s `0xEFFFFFFF`. `360B4`'s
`6FE14` child tail is not this cut.

Do not force script PC, call `2F7D8`, pre-create type 2, or
plant D20C / D2A4 / BE834. Type 2 still needs `0x6F` for a
body. New-game type-0 `0x09` subop `0x0B` persist`[0x4A]==39`
is false, so `0x05` skips to `+0x608` and does **not** send
`0x1C(2,0,0xB)`. That mailbox is the persist-39 arm.

## Verify

```text
python3 pc_port/tools/pe_btl124_type1_type2_oracle.py
PE_TEST_FILTER=BTL124 ./pc_port/build/pe-native-tests
```
