# PE-CH1 — func_80030220 opcode 0x5A slot tagged setter

Native translation of the post-`0x6F` slot `0x5A` store. Matching
`src/` C was not added: this worktree has no `asm/`, no era `cc1`,
and no extracted SLUS.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x80030220..0x80030530  (197 words, 0x314)
file        0x20A20
yaml        [0x20210, asm]
table       D_80010C90[tag-40], 85 entries; reject tag-40 >= 85
jal         0x800181B4 (0x5A) and 0x80018288 (0xCE)
wrapper     0x80018164 when actor+0x0C != 0
```

## Contract

```text
slot = *actor
idx  = (tag & 0xFF) - 40
if idx >= 85: return
jr D_80010C90[idx]
```

m0005i first-play (BATTLE_RESOURCES / FORMATION_SELECTION):

| tag | value | store |
|---|---|---|
| 40 | 0 | `sh` slot+0x0E |
| 41 | 1 | `sb` slot+0x04 |
| 42 | 2 | `sb` slot+0x05 |
| 50 | 1333 | `sh` slot+0xB0 |
| 51 | 1332 | `sh` slot+0xB2 |
| 52 | 1334 | `sh` slot+0xB4 |

Remaining table entries are ROM-backed (simple stores, bitfields on
word0 / +0xCC, tag 80 sb+0xA4 plus constant +0xA6, or nop).

## Verify

```text
python3 pc_port/tools/pe_ch1_30220_oracle.py
# from pc_port/build: PE_TEST_FILTER=30220 ./pe-native-tests
```

Oracle: 197/197 ROM words + JT BTL1 tags 40–42/50–52 (tag-50 `sh +0xB0`
is the delay slot of `j` at WORDS[101]; store is WORDS[102]).
Native tests: 5 focused `30220_*` plus full suite **641/641**.
Tag 48/49 jump to the `jr $ra` epilogue (true nop).
