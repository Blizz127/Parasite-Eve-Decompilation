# PE-BTL66 — Writer A command table and 1A4AC clip ticker

Type-0 persist==0x27 after `0xB8` polls `scratch[0x12]==0`.
The authentic writer is type-2 mailbox payload `0x1A`, which
runs already-ported `0x2E(0x17)` / `0x4E(0)` / `0x2F(0x33)` /
`0x30` and then `0x0A scratch[0x12]=1`.

`0x30` waits until `actor+0x16 == actor+0x12`. Empty `B0E98`
made `1A680` set `+0x0F=0xFF`, so `+0x12=0x33` and the wait
never finished. The missing publisher is 6B4F8 hdr+0x10
Writer A, not an invented `+0x16` increment.

## Writer A — ROM `0x8006B828..0x8006B894`

Before the 12574 stores. `count = *(hdr+0x10)>>22`,
`rec = chunk2 + (word & 0x3FFFFF)`, 12-byte records:

* `lbu` type `rec+0x0B`
* `lbu` command `rec+7`
* pointer `rec+4 & 0x00FFFFFF`
* store `chunk2+ptr` at `overlay+0x1C0 + type*192 + cmd*4`

hdr+0x0C B0E70 bind remains not this cut.

Live m0005i: 21 records. Type-2 command `0x17` pointer
`0x1F3A8`, `byte+2 = 51` → `1A680` sets `+0x0F = 50`.

## `func_8001A4AC`

117 words `0x8001A4AC..0x8001A680`, SHA-256 `53d93b57…2174`.
35558 jals it at `35B84` (D254 when `D1A0&0x100`) and `35BEC`
(D20C walk). `+0x14 += +0x1C` (125E0 plants `0x10000`) while
bit `0x200` is set, clamping to `+0x12<<16`. That is the
`+0x16` ticker. `6A318` / `1A784` only when `+0x18C != 0`;
125E0 zeros that pointer.
