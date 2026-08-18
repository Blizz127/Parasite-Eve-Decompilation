# PE-BTL6 — func_8003D050 remainder named cuts + live 6698C / 3C5D8

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. `jal 794C4`, `3D834` callees, `andi 0xFC`,
mode 7, and `0x55` completion were not invented.

## Exclusive cuts inside 3D050 (505 words, SHA-256 `50b5ff75…`)

| Cut | VA | Words | SHA-256 prefix |
|---|---|---|---|
| prefix (prior) | `0x8003D078..0x8003D0D4` | — | dest+0/4/8/C/10 |
| ptr14 | `0x8003D36C..0x8003D394` | 10 | `825bccd2…` |
| post-3D94C-skip | `0x8003D5D8..0x8003D750` | 94 | `93d42cd9…` |
| epilogue | `0x8003D76C..0x8003D834` | 50 | `cc904e70…` |

Three jals only:

| Site | Target | Live EE=13? |
|---|---|---|
| `0x8003D5D0` | `func_8003D94C` | **SKIPPED** — `blez` on stack halfword `0x54(sp)=0` |
| `0x8003D750` | `func_800794C4` | **LIVE** — 888 words, five callees; not this cut |
| `0x8003D764` | `func_8003C5D8` | **LIVE** — 24 words, zero callees; ported |

`t8==0` skip at `0x8003D338` and the packet-fill fall-through both
reach `0x8003D36C` with `a1 = dest+0x10 + (lhu obj+8/+A/+C/+E)*12`.
Then `+0x14/+0x18/+0x1C/+0x20`. Post-skip stores `+0x70/+0x72/+0x80/+0x84`,
optional stream loops when `lbu(obj+2)!=0`, then zeros `+0x2C..+0x30`
and `+0x32=1`. Stop before `jal 794C4`.

Epilogue after `3C5D8` (delay `sb -1 → +0x8C` is the caller's store):
`+0x90/91/92=0x80`, `+0x9E=1`, `+0x9C/+0x28/+0x2A/+0x24=0`,
`+0x9F=D_8009CDDC`, `obj+0x1A = lhu(obj+6)-s2`, `+0x6E`, `+0xA6/A7` pair,
`+0xB0=0`.

## `func_8003C5D8` — 24 words, SHA-256 `237a6038…`

`0x8003C5D8..0x8003C638`. `a1==0` forces divisor 1; else
`sb a1 → +0x8D`, `sb (128/a1) → +0x8E/+0x8F/+0x93`. Live a1=50 → 2.

## `func_8006698C` live leaf — 117 words, SHA-256 `71478d80…`

The 215-word window `0x8006698C..0x80066CE8` is **four** `jr` leaves.
EE=13 jals only the first: exclusive end `0x80066B60`. Fills
`D_800BEA40` / `D_800BEA60` from dest+0x88/89/8A and
`D_800BD025/26/27`. Trailing five `mtc2` into C2DR0-4 are GTE data
registers (no guest-RAM address) and are not invented.

## `func_8003D834` bound correction

Not 489 words. First `jr` at `0x8003D944`; exclusive end `0x8003D94C`
(70 words, SHA-256 `520529b0…`). The 489-word span walks into
`func_8003D94C`. Live EE=13 `a1=0` skips the first three jals and
always hits `3A088`, `3DFD8`, `3B97C`, `3BCE0`×2. Those callees are
not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl6_3d050_remainder_oracle.py
python3 pc_port/tools/pe_btl6_ee13_oracle.py
python3 pc_port/tools/pe_btl5_overlay_wait_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
```

## `func_8003DFD8` live copy — 51 words, SHA-256 `c3062318…`

Exclusive `0x8003DFD8..0x8003E0A4` (first jr). Live 3D834 a1==0:
`jal 3DFD8(0x800B1638, dest+0x34, 1)`. Copies `count` records:
halfwords `+0..+16`, words `+0x14/+0x18/+0x1C`; `+0x12` untouched.

## `func_800794C4` first leaf — 163 words, SHA-256 `19a788c4…`

Exclusive `0x800794C4..0x80079750`. Zero callees. Live EE=13 angles
at dest+0x2C are 0; `D_800966EC[0]=0x10000000` yields the 4096
identity at dest+0x34. Packed later leaves / `EnterCriticalSection`
are not this cut.

STOP: 3D834 a1==0 callees are in `pe-btl6-3d834-callees`.
Next `0x8003A3B4`. Do not `andi 0xFC`.
