# PE-BTL60 — opcode 0x87 / 0x31

## 0x87 / `0x80018F0C`

18 words `0x80018F0C..0x80018F54`, SHA-256
`cb2c045fd48813b66f79f29a11c45d26c3394c02ac40601fb63740f02baed649`.
`jal 0x80066BD8` with five halfword args. `v0=1`.

## `func_80066BD8`

41 words `0x80066BD8..0x80066C7C`, SHA-256
`ebcd3c1a1457163e25b0a72b07729e8b8bd418bfe342f8e7e3e97174c25930cc`.
Writes `CFE8/EA/EC` from a1/a2/a3, `CFEE=2`, `CFF6=a0`, `CFF8=0`,
snapshot to `CFF0/F2/F4`. `CFEF = a4` when `a4<4`, else 1.

Live type-0 Watch arm `+0x1020`: `(0x3C, 0xFF, 0xFF, 0xFF, 1)` then
`0x9C` wait. Then `persist[1]=5`, `persist[0x4A]=0x26`.

## 0x31 / `0x80017BB4`

Already recovered for BTL1. The named cut now stores any token except
the unported `0xA9400048` arm. Live Watch arm token is `0xA80663C8`.
`v0=0` (VM yield). `D1A0 |= 0x2000`, `D280 = token`.
