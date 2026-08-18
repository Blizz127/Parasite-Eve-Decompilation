# PE-BTL89 — 0xC6 / 13300 command-wait

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`13300` — 58 words `0x80013300..0x800133E8`,
SHA-256 `27e1042f…81a9`. `D_800910A0[0xC6]`.
Sole jal `1A680`.

First visit (`task+8 & 0x20 == 0`): set that bit,
`1A680(D2F0, lhu *arg0)`, `actor+0x98 &= ~0x100`,
rewind `CE00` by `0xC`, `task+0x10 = 1`, `v0=0`.

Later visits: `actor+0x0F==0` clears the bit and
returns 1. Else `sltu(+0x14,+0x18)` when
`+0x1C>=0`, or swapped when `+0x1C<0`. Not ready
rewinds and yields. Ready clears bit `0x20`.

Type-0 `+0x12D4` imm `0x1D` is the first unported
opcode on that stream. The persist!=39 arm parks
on `0x20` (`task+8 |= 0x10`) first. The EXE has
no `andi 0xFFEF` of `task+8`; that wake stays a
research target. Do not force the sleep bit.

## Verify

```text
python3 pc_port/tools/pe_btl89_13300_c6_oracle.py
PE_TEST_FILTER=BTL89 ./pc_port/build/pe-native-tests
```
