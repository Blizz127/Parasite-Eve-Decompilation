# PE-BTL68 — 3999C indexes `*codep`, jalrs 710A4/7136C

BTL67 treated `actor+0` as the pad-table index. ROM `0x800399C0`
is `lw $v0, 0($s2)` with `s2 = a2` (`0x800399AC`). That is
`table[*codep]`. `2F76C`'s `0x800B8A20` at `actor+0` is the
0x70 record pointer 7136C dereferences; it is not the 3999C
index. **REJECTED:** host-guard on `actor+0 >= 64`.

Live type-0 after `0x2E(0x15)` / `0x3F` writes code `0x15` = 21.
`table[21] = 0x800942F0`:

| rec | flags | m1 | m2 | fn |
|---|---|---|---|---|
| +0 | 0x3 | 1 | 0x78 | 7136C |
| +16 | 0x2 | 0 | 0x78 | 710A4 |
| +32 | 0 | 0 | 0 | 0 |

`flags=0x3` needs `D26C&1==1` and `D26C&0x78!=0` (Circle + d-pad).
`flags=0x2` needs `D26C&0x78!=0` (d-pad). `D26C=0` jalrs nothing.

`3EB04` / `A76F0`: bit0 = Circle, bits 3–6 = Up/Right/Down/Left.
Do not invent pad. Idle `D26C=0` is authentic until a direction
is held.

`710A4` (178w) with `D1A0&2` jals `7136C`; else binds command
`0x16` and walks at speed `0x50000`. `7136C` (206w) binds `0x17`
at `0x168000` when `D1A0&2` is clear. Both digital tails jal
`78934` (88w) against walk matrix `D_800BD000`. Analog
(`BE9A0&0xF000==0x7000`) is not this cut. `71034` / `716A4` /
`71754` are not this cut.
